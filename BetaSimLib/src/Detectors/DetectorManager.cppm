//
// Created by Sonora on 18.06.2026.
//
export module BetaSimLib.Detectors.DetectorManager;

import std;
import Geant4.Externals;

import BetaSimLib.Models.Experiment;
import BetaSimLib.Detectors.DetectorWrapper;
import BetaSimLib.Detectors.StartDetector;
import BetaSimLib.Concepts.Detectors;

export namespace BetaSimLib::Detectors {

class DetectorManager {
#pragma region Constructors/Destructors

public:
    DetectorManager()
        : ownedWrapper(std::make_unique<DetectorWrapper>()),
          wrapperView(ownedWrapper.get()) {
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
        if (!wrapperView) {
            Geant4::G4Exception(
                "DetectorManager",
                "NoDetectorWrapper",
                Geant4::G4ExceptionSeverity::FatalException,
                "DetectorWrapper is null."
            );

            return;
        }

        wrapperView->SetBuilder(std::move(detector));
    }

    void ApplyConfig(std::shared_ptr<const Models::BaseExperimentConfig> config) {
        currentConfig = std::move(config);

        if (!currentConfig) {
            SetDetector(std::make_unique<StartDetector>());
            return;
        }

        switch (currentConfig->type) {
            case Models::ExpType::Stack:
                // TODO:
                // Здесь потом поставишь свой реальный builder:
                //
                // SetDetector(
                //     std::make_unique<StackDetectorConstruction>(currentConfig)
                // );
                //
                // Пока временно оставляем StartDetector.
                SetDetector(std::make_unique<StartDetector>());
                break;

            case Models::ExpType::None:
            default:
                SetDetector(std::make_unique<StartDetector>());
                break;
        }
    }

    DetectorWrapper* GetCurrentDetectorPointer() {
        return wrapperView;
    }

    const DetectorWrapper* GetCurrentDetectorPointer() const {
        return wrapperView;
    }

    Geant4::G4VUserDetectorConstruction* ReleaseDetectorToGeant4() {
        return ownedWrapper.release();
    }

#pragma endregion

#pragma region Variables

private:
    std::unique_ptr<DetectorWrapper> ownedWrapper;
    DetectorWrapper* wrapperView = nullptr;

    std::shared_ptr<const Models::BaseExperimentConfig> currentConfig;

#pragma endregion
};

} // namespace BetaSimLib::Detectors