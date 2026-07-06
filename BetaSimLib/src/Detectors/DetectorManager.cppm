//
// Created by Sonora on 18.06.2026.
//
export module BetaSimLib.Detectors.DetectorManager;

import std;
import Geant4.Externals;

import BetaSimLib.Models.Experiment;
import BetaSimLib.Concepts.Detectors;

import BetaSimLib.Detectors.DetectorWrapper;
import BetaSimLib.Detectors.StartDetector;
import BetaSimLib.Detectors.BaseDetectorConstruction;

export namespace BetaSimLib::Detectors {

class DetectorManager {
#pragma region Constructors/Destructors

public:
    DetectorManager()
        : detWrapper(std::make_unique<DetectorWrapper>()) {
    }

    ~DetectorManager() = default;

    DetectorManager(const DetectorManager&) = delete;
    DetectorManager& operator=(const DetectorManager&) = delete;

    DetectorManager(DetectorManager&&) = delete;
    DetectorManager& operator=(DetectorManager&&) = delete;

#pragma endregion

#pragma region Methods

public:
    template<Concepts::Detectors::DetectorConstructionConcept Detector>
    void SetDetector(std::unique_ptr<Detector> detector) {
        if (!detWrapper) {
            Geant4::G4Exception(
                "DetectorManager",
                "NoDetectorWrapper",
                Geant4::G4ExceptionSeverity::FatalException,
                "DetectorWrapper is null."
            );

            return;
        }

        detWrapper->SetBuilder(std::move(detector));
    }

    void ApplyConfig(std::shared_ptr<const Models::BaseExperimentConfig> newConfig) {
        currentConfig = std::move(newConfig);

        if (!currentConfig) {
            SetDetector(
                std::make_unique<StartDetector>()
            );

            return;
        }

        switch (currentConfig->type) {
            case Models::ExpType::Stack:
                SetDetector(
                    std::make_unique<BaseDetectorConstruction>(currentConfig)
                );
                break;

            case Models::ExpType::None:
            default:
                SetDetector(
                    std::make_unique<StartDetector>()
                );
                break;
        }
    }

    DetectorWrapper* GetCurrentDetectorPointer() {
        return detWrapper.get();
    }

    const DetectorWrapper* GetCurrentDetectorPointer() const {
        return detWrapper.get();
    }

#pragma endregion

#pragma region Variables

private:
    std::unique_ptr<DetectorWrapper> detWrapper;
    std::shared_ptr<const Models::BaseExperimentConfig> currentConfig;

#pragma endregion
};

} // namespace BetaSimLib::Detectors