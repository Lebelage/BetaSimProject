export module BetaSimLib.SensitiveDetectors.BaseSD;

import std;

import Geant4.Externals;

import BetaSimLib.SensitiveDetectors.Scoring.DetectorHit;
import BetaSimLib.Statistics.SimulationStatisticsService;
import BetaSimLib.Materials.ExtendedMaterialService;

export namespace BetaSimLib::SensitiveDetectors {

class BaseSD final : public Geant4::G4VSensitiveDetector {
public:
    explicit BaseSD(const Geant4::G4String& name)
        : Geant4::G4VSensitiveDetector(name) {
        collectionName.insert("DetectorHitsCollection");
    }

    ~BaseSD() override = default;

    BaseSD(const BaseSD&) = delete;
    BaseSD& operator=(const BaseSD&) = delete;

    BaseSD(BaseSD&&) = delete;
    BaseSD& operator=(BaseSD&&) = delete;

public:
    void Initialize(Geant4::G4HCofThisEvent* hitCollectionOfEvent) override {
        hitsCollection = new Scoring::DetectorHitsCollection(
            SensitiveDetectorName,
            collectionName[0]
        );

        if (hitsCollectionId < 0) {
            hitsCollectionId =
                Geant4::G4SDManager::GetSDMpointer()
                    ->GetCollectionID(
                        SensitiveDetectorName + "/" + collectionName[0]
                    );
        }

        hitCollectionOfEvent->AddHitsCollection(
            hitsCollectionId,
            hitsCollection
        );

        recordedElectronTrackIds.clear();
    }

    Geant4::G4bool ProcessHits(
        Geant4::G4Step* step,
        Geant4::G4TouchableHistory*
    ) override {
        if (!step) {
            return false;
        }

        auto* preStepPoint = step->GetPreStepPoint();

        if (!preStepPoint) {
            return false;
        }

        auto* touchable = preStepPoint->GetTouchable();

        if (!touchable) {
            return false;
        }

        auto* track = step->GetTrack();

        if (!track) {
            return false;
        }

        const auto energyDeposit = step->GetTotalEnergyDeposit();

        auto* material = preStepPoint->GetMaterial();

        const std::string materialName =
            material
                ? std::string(material->GetName())
                : std::string("Unknown");

        const auto layerId = touchable->GetCopyNumber();

        const auto layerName =
            preStepPoint->GetPhysicalVolume()
                ? std::string(preStepPoint->GetPhysicalVolume()->GetName())
                : std::string("UnknownLayer");

        const auto electronHolePairs =
            CalculateElectronHolePairs(materialName, energyDeposit);

        const bool detectorEntry =
            IsElectronDetectorEntry(step, track);

        const auto kineticEnergyBeforeStep =
            preStepPoint->GetKineticEnergy();

        if (detectorEntry) {
            BetaSimLib::Statistics::SimulationStatisticsService::Instance()
                .RecordElectronEntryEnergy(kineticEnergyBeforeStep);
        }

        if (energyDeposit > 0.0 || electronHolePairs > 0.0 || detectorEntry) {
            auto* hit = new Scoring::DetectorHit();

            hit->SetTrackId(track->GetTrackID());
            hit->SetLayerId(layerId);
            hit->SetMaterialName(materialName);
            hit->SetEnergyDeposit(energyDeposit);
            hit->SetElectronHolePairs(electronHolePairs);
            hit->SetKineticEnergyBeforeStep(kineticEnergyBeforeStep);
            hit->SetDetectorEntry(detectorEntry);
            hit->SetPosition(preStepPoint->GetPosition());

            hitsCollection->insert(hit);
        }

        if (energyDeposit > 0.0 || electronHolePairs > 0.0) {
            const auto* currentEvent =
                Geant4::G4RunManager::GetRunManager()
                    ->GetCurrentEvent();

            const auto eventId =
                currentEvent ? currentEvent->GetEventID() : -1;

            BetaSimLib::Statistics::SimulationStatisticsService::Instance()
                .RecordDetectorStep(
                    static_cast<std::uint64_t>(eventId),
                    layerId,
                    layerName,
                    materialName,
                    energyDeposit,
                    electronHolePairs
                );
        }

        return true;
    }

    void EndOfEvent(Geant4::G4HCofThisEvent*) override {
        if (!hitsCollection) {
            return;
        }

        double totalEnergyDeposit = 0.0;
        double totalElectronHolePairs = 0.0;

        std::map<int, double> layerEnergyDeposit;
        std::map<int, double> layerElectronHolePairs;

        for (int i = 0; i < hitsCollection->entries(); ++i) {
            auto* hit = (*hitsCollection)[i];

            if (!hit) {
                continue;
            }

            totalEnergyDeposit += hit->GetEnergyDeposit();
            totalElectronHolePairs += hit->GetElectronHolePairs();

            layerEnergyDeposit[hit->GetLayerId()] +=
                hit->GetEnergyDeposit();

            layerElectronHolePairs[hit->GetLayerId()] +=
                hit->GetElectronHolePairs();
        }

        if (totalEnergyDeposit <= 0.0 && totalElectronHolePairs <= 0.0) {
            return;
        }

        Geant4::cout()
            << "[DetectorSD] Event total:"
            << " Edep = " << totalEnergyDeposit / Geant4::keV << " keV,"
            << " EHP = " << totalElectronHolePairs
            << Geant4::endl;

        for (const auto& [layerId, edep] : layerEnergyDeposit) {
            Geant4::cout()
                << "  layer " << layerId
                << " Edep = " << edep / Geant4::keV << " keV,"
                << " EHP = " << layerElectronHolePairs[layerId]
                << Geant4::endl;
        }
    }

private:
    bool IsElectronDetectorEntry(
        Geant4::G4Step* step,
        Geant4::G4Track* track
    ) {
        if (!step || !track) {
            return false;
        }

        auto* particle = track->GetDefinition();

        if (!particle) {
            return false;
        }

        if (particle->GetParticleName() != "e-") {
            return false;
        }

        auto* preStepPoint = step->GetPreStepPoint();

        if (!preStepPoint) {
            return false;
        }

        if (preStepPoint->GetStepStatus() != Geant4::fGeomBoundary) {
            return false;
        }

        const auto trackId = track->GetTrackID();

        if (recordedElectronTrackIds.contains(trackId)) {
            return false;
        }

        recordedElectronTrackIds.insert(trackId);

        return true;
    }

    double CalculateElectronHolePairs(
        const std::string& materialName,
        double energyDeposit
    ) const {
        if (energyDeposit <= 0.0) {
            return 0.0;
        }

        const auto pairCreationEnergy =
            ResolvePairCreationEnergy(materialName);

        if (pairCreationEnergy <= 0.0) {
            return 0.0;
        }

        return energyDeposit / pairCreationEnergy;
    }

    double ResolvePairCreationEnergy(
        const std::string& materialName
    ) const {

        if (materialName == "GaN") {
            return 8.9 * Geant4::eV;
        }

        auto optExtMat =
            BetaSimLib::Materials::ExtendedMaterialService::Instance()
                .Get(materialName);

        if (optExtMat.has_value() && optExtMat.value() != nullptr) {
            const auto eg = optExtMat.value()->GetEg();

            if (eg > 0.0f) {
                return 3.0 * static_cast<double>(eg) * Geant4::eV;
            }
        }

        return 0.0;
    }

private:
    Scoring::DetectorHitsCollection* hitsCollection = nullptr;
    int hitsCollectionId = -1;

    std::unordered_set<int> recordedElectronTrackIds;
};

} // namespace BetaSimLib::SensitiveDetectors