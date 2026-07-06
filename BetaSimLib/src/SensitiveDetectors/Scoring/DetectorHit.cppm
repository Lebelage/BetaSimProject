export module BetaSimLib.SensitiveDetectors.Scoring.DetectorHit;

import std;
import Geant4.Externals;

export namespace BetaSimLib::SensitiveDetectors::Scoring {

class DetectorHit final : public Geant4::G4VHit {
public:
    DetectorHit() = default;
    ~DetectorHit() override = default;

    DetectorHit(const DetectorHit&) = default;
    DetectorHit& operator=(const DetectorHit&) = default;

public:
    void SetTrackId(int value) {
        trackId = value;
    }

    void SetLayerId(int value) {
        layerId = value;
    }

    void SetMaterialName(std::string value) {
        materialName = std::move(value);
    }

    void SetEnergyDeposit(double value) {
        energyDeposit = value;
    }

    void SetElectronHolePairs(double value) {
        electronHolePairs = value;
    }

    void SetKineticEnergyBeforeStep(double value) {
        kineticEnergyBeforeStep = value;
    }

    void SetDetectorEntry(bool value) {
        detectorEntry = value;
    }

    void SetPosition(const Geant4::G4ThreeVector& value) {
        position = value;
    }

    int GetTrackId() const {
        return trackId;
    }

    int GetLayerId() const {
        return layerId;
    }

    const std::string& GetMaterialName() const {
        return materialName;
    }

    double GetEnergyDeposit() const {
        return energyDeposit;
    }

    double GetElectronHolePairs() const {
        return electronHolePairs;
    }

    double GetKineticEnergyBeforeStep() const {
        return kineticEnergyBeforeStep;
    }

    bool IsDetectorEntry() const {
        return detectorEntry;
    }

    const Geant4::G4ThreeVector& GetPosition() const {
        return position;
    }

private:
    int trackId = -1;
    int layerId = -1;

    std::string materialName;

    double energyDeposit = 0.0;
    double electronHolePairs = 0.0;
    double kineticEnergyBeforeStep = 0.0;

    bool detectorEntry = false;

    Geant4::G4ThreeVector position;
};

using DetectorHitsCollection =
    Geant4::G4THitsCollection<DetectorHit>;

}