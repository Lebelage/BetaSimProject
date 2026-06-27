//
// Created by Sonora on 18.06.2026.
//
export module BetaSimLib.Detectors.DetectorWrapper;

import std;
import Geant4.Externals;
import BetaSimLib.Concepts.Detectors;

export namespace BetaSimLib::Detectors {

class DetectorWrapper final : public Geant4::G4VUserDetectorConstruction {
#pragma region Constructors/Destructors

public:
    DetectorWrapper() = default;
    ~DetectorWrapper() override = default;

    DetectorWrapper(const DetectorWrapper&) = delete;
    DetectorWrapper& operator=(const DetectorWrapper&) = delete;

    DetectorWrapper(DetectorWrapper&&) = delete;
    DetectorWrapper& operator=(DetectorWrapper&&) = delete;

#pragma endregion

#pragma region Methods

public:
    template<Concepts::Detectors::DetectorConstructionConcept Detector>
    void SetBuilder(std::unique_ptr<Detector> builder) {
        currentBuilder = std::move(builder);
    }

    bool HasBuilder() const {
        return currentBuilder != nullptr;
    }

    Geant4::G4VPhysicalVolume* Construct() override {
        if (!currentBuilder) {
            Geant4::G4Exception(
                "DetectorWrapper",
                "NoDetectorBuilder",
                Geant4::G4ExceptionSeverity::FatalException,
                "Detector builder is not set."
            );

            return nullptr;
        }

        auto* world = currentBuilder->Construct();

        if (!world) {
            Geant4::G4Exception(
                "DetectorWrapper",
                "NullWorld",
                Geant4::G4ExceptionSeverity::FatalException,
                "Detector builder returned nullptr world."
            );

            return nullptr;
        }

        return world;
    }

    void ConstructSDandField() override {
        if (currentBuilder) {
            currentBuilder->ConstructSDandField();
        }
    }

#pragma endregion

#pragma region Variables

private:
    std::unique_ptr<Geant4::G4VUserDetectorConstruction> currentBuilder;

#pragma endregion
};

} // namespace BetaSimLib::Detectors