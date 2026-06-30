//
// Created by Sonora on 27.06.2026.
//
export module BetaSimLib.SensitiveDetectors.BaseSD;

import std;
import Geant4.Externals;

import BetaSimLib.SensitiveDetectors.Scoring.DetectorHit;
import BetaSimLib.Statistics.SimulationStatisticsService;
import BetaSimLib.Materials.ExtendedMaterialService;

export namespace BetaSimLib::SensitiveDetectors {

class BaseSD final : public Geant4::G4VSensitiveDetector {
#pragma region Constructors/Destructors

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

#pragma endregion

#pragma region Configuration

public:
    void Configure(
        double detectorThickness,
        double zBinWidth,
        double maxSpectrumEnergy,
        double spectrumBinWidth
    ) {
        if (zBinWidth <= 0.0) {
            zBinWidth = 1.0 * Geant4::nm;
        }

        if (detectorThickness <= 0.0) {
            detectorThickness = zBinWidth;
        }

        if (spectrumBinWidth <= 0.0) {
            spectrumBinWidth = 0.1 * Geant4::keV;
        }

        if (maxSpectrumEnergy <= 0.0) {
            maxSpectrumEnergy = 100.0 * Geant4::keV;
        }

        this->detectorThickness = detectorThickness;
        this->zBinWidth = zBinWidth;
        this->maxSpectrumEnergy = maxSpectrumEnergy;
        this->spectrumBinWidth = spectrumBinWidth;

        auto& stats =
            BetaSimLib::Statistics::SimulationStatisticsService::Instance();

        stats.ConfigureDepthProfile(
            detectorThickness,
            zBinWidth
        );

        const auto spectrumBinCount =
            static_cast<std::size_t>(
                std::ceil(maxSpectrumEnergy / spectrumBinWidth)
            );

        stats.ConfigureElectronSpectrum(
            0.0,
            maxSpectrumEnergy,
            std::max<std::size_t>(1, spectrumBinCount)
        );
    }

#pragma endregion

#pragma region Geant4 SD Methods

public:
    void Initialize(
        Geant4::G4HCofThisEvent* hitCollectionOfEvent
    ) override {
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

        if (hitCollectionOfEvent) {
            hitCollectionOfEvent->AddHitsCollection(
                hitsCollectionId,
                hitsCollection
            );
        }

        processedElectronTrackIds.clear();
    }

    Geant4::G4bool ProcessHits(
        Geant4::G4Step* step,
        Geant4::G4TouchableHistory*
    ) override {
        if (!step) {
            return false;
        }

        auto* preStepPoint = step->GetPreStepPoint();
        auto* postStepPoint = step->GetPostStepPoint();

        if (!preStepPoint || !postStepPoint) {
            return false;
        }

        auto* track = step->GetTrack();

        if (!track) {
            return false;
        }

        CollectElectronEntrySpectrum(track, preStepPoint);
        CollectReflectionInfo(track, postStepPoint);
        CollectEnergyDeposit(step, track, preStepPoint, postStepPoint);

        return true;
    }

    void EndOfEvent(
        Geant4::G4HCofThisEvent*
    ) override {
        BetaSimLib::Statistics::SimulationStatisticsService::Instance()
            .RecordEvent();

    }

#pragma endregion

#pragma region Collectors

private:
    void CollectElectronEntrySpectrum(
        Geant4::G4Track* track,
        Geant4::G4StepPoint* preStepPoint
    ) {
        if (!IsElectron(track)) {
            return;
        }

        if (!preStepPoint) {
            return;
        }

        // Электрон только что вошёл в sensitive detector volume.
        if (preStepPoint->GetStepStatus() != Geant4::fGeomBoundary) {
            return;
        }

        if (preStepPoint->GetMomentumDirection().z() >= 0.0) {
            return;
        }

        if (WasBornInsideDetector(track)) {
            return;
        }

        const auto trackId = track->GetTrackID();

        if (processedElectronTrackIds.contains(trackId)) {
            return;
        }

        processedElectronTrackIds.insert(trackId);

        const auto kineticEnergy =
            preStepPoint->GetKineticEnergy();

        BetaSimLib::Statistics::SimulationStatisticsService::Instance()
            .RecordElectronEntryEnergy(kineticEnergy);
    }

    void CollectReflectionInfo(
        Geant4::G4Track* track,
        Geant4::G4StepPoint* postStepPoint
    ) {
        if (!IsElectron(track)) {
            return;
        }

        if (!postStepPoint) {
            return;
        }

        if (postStepPoint->GetStepStatus() != Geant4::fGeomBoundary) {
            return;
        }

        auto touchable = postStepPoint->GetTouchableHandle();

        if (!touchable || !touchable->GetVolume()) {
            return;
        }

        auto* postVolume = touchable->GetVolume();

        if (!postVolume) {
            return;
        }

        const std::string postVolumeName =
            postVolume->GetName();

        
        if (!IsDetectorPhysicalVolumeName(postVolumeName)) {
            if (postStepPoint->GetMomentumDirection().z() > 0.0) {
                BetaSimLib::Statistics::SimulationStatisticsService::Instance()
                    .RecordReflected();

                track->SetTrackStatus(Geant4::fStopAndKill);
            }
        }
    }

    void CollectEnergyDeposit(
        Geant4::G4Step* step,
        Geant4::G4Track* track,
        Geant4::G4StepPoint* preStepPoint,
        Geant4::G4StepPoint* postStepPoint
    ) {
        if (!step || !preStepPoint || !postStepPoint) {
            return;
        }

        const auto energyDeposit =
            step->GetTotalEnergyDeposit();

        if (energyDeposit <= 0.0) {
            return;
        }

        auto* material =
            preStepPoint->GetMaterial();

        const std::string materialName =
            material
                ? std::string(material->GetName())
                : std::string("Unknown");

        auto* physicalVolume =
            preStepPoint->GetPhysicalVolume();

        const std::string layerName =
            physicalVolume
                ? std::string(physicalVolume->GetName())
                : std::string("UnknownLayer");

        auto* touchable =
            preStepPoint->GetTouchable();

        const int layerId =
            touchable
                ? touchable->GetCopyNumber()
                : -1;

        const auto electronHolePairs =
            CalculateElectronHolePairs(
                materialName,
                energyDeposit
            );

        const auto kineticEnergyBeforeStep =
            preStepPoint->GetKineticEnergy();

        const auto depth =
            CalculateDetectorDepth(
                preStepPoint,
                postStepPoint
            );

        auto* currentEvent =
            Geant4::G4RunManager::GetRunManager()
                ->GetCurrentEvent();

        const auto eventId =
            currentEvent
                ? currentEvent->GetEventID()
                : -1;

        auto& stats =
            BetaSimLib::Statistics::SimulationStatisticsService::Instance();

        stats.RecordDetectorStep(
            static_cast<std::uint64_t>(eventId),
            layerId,
            layerName,
            materialName,
            energyDeposit,
            electronHolePairs
        );

        stats.RecordDepthProfileStep(
            depth,
            layerId,
            layerName,
            materialName,
            energyDeposit,
            electronHolePairs
        );

        if (hitsCollection) {
            auto* hit = new Scoring::DetectorHit();

            hit->SetTrackId(track ? track->GetTrackID() : -1);
            hit->SetLayerId(layerId);
            hit->SetMaterialName(materialName);
            hit->SetEnergyDeposit(energyDeposit);
            hit->SetElectronHolePairs(electronHolePairs);
            hit->SetKineticEnergyBeforeStep(kineticEnergyBeforeStep);
            hit->SetDetectorEntry(false);
            hit->SetPosition(preStepPoint->GetPosition());

            hitsCollection->insert(hit);
        }
    }

#pragma endregion

#pragma region Helpers

private:
    bool IsElectron(
        Geant4::G4Track* track
    ) const {
        if (!track) {
            return false;
        }

        auto* particle =
            track->GetDefinition();

        if (!particle) {
            return false;
        }

        return particle->GetParticleName() == "e-";
    }

    bool WasBornInsideDetector(
        Geant4::G4Track* track
    ) const {
        if (!track) {
            return false;
        }

        auto* vertexLogicalVolume =
            track->GetLogicalVolumeAtVertex();

        if (!vertexLogicalVolume) {
            return false;
        }

        const std::string vertexVolumeName =
            vertexLogicalVolume->GetName();

        return IsDetectorLogicalVolumeName(vertexVolumeName);
    }

    bool IsDetectorPhysicalVolumeName(
        const std::string& volumeName
    ) const {
        return volumeName.rfind("DetLayer_Phys_", 0) == 0;
    }

    bool IsDetectorLogicalVolumeName(
        const std::string& volumeName
    ) const {
        return volumeName.rfind("DetLayer_Logic_", 0) == 0;
    }

    double CalculateDetectorDepth(
        Geant4::G4StepPoint* preStepPoint,
        Geant4::G4StepPoint* postStepPoint
    ) const {
        if (!preStepPoint || !postStepPoint) {
            return 0.0;
        }

        const auto zPre =
            preStepPoint->GetPosition().z();

        const auto zPost =
            postStepPoint->GetPosition().z();

        const auto zMid =
            0.5 * (zPre + zPost);

        // Detector строится от Z = 0 в сторону -Z.
        // Поэтому физическая глубина = -z.
        const auto depth =
            -zMid;

        if (depth < 0.0) {
            return 0.0;
        }

        if (detectorThickness > 0.0 && depth > detectorThickness) {
            return detectorThickness;
        }

        return depth;
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
        // Старый подход:
        // E_EHP_eV = 2.8 * Eg + 0.6
        //
        // Для GaN с Eg около 3.4 eV:
        // 2.8 * 3.4 + 0.6 ~= 10.12 eV.

        auto optExtMat =
            BetaSimLib::Materials::ExtendedMaterialService::Instance()
                .Get(materialName);

        if (optExtMat.has_value() && optExtMat.value() != nullptr) {
            const auto eg =
                optExtMat.value()->GetEg();

            if (eg > 0.0f) {
                return (
                    2.8 * static_cast<double>(eg) + 0.6
                ) * Geant4::eV;
            }
        }

        if (materialName == "GaN") {
            return 10.12 * Geant4::eV;
        }

        // Для металлов и неизвестных материалов ЭДП не считаем.
        return 0.0;
    }

#pragma endregion

#pragma region Variables

private:
    Scoring::DetectorHitsCollection* hitsCollection = nullptr;
    int hitsCollectionId = -1;

    std::unordered_set<int> processedElectronTrackIds;

    double detectorThickness = 0.0;
    double zBinWidth = 1.0 * Geant4::nm;

    double maxSpectrumEnergy = 100.0 * Geant4::keV;
    double spectrumBinWidth = 0.1 * Geant4::keV;

#pragma endregion
};

} // namespace BetaSimLib::SensitiveDetectors