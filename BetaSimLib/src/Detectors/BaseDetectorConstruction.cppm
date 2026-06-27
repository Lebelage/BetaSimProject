export module BetaSimLib.Detectors.BaseDetectorConstruction;

import std;
import Geant4.Externals;

import BetaSimLib.Models.Experiment;
import BetaSimLib.Materials.ExtendedMaterialService;
import BetaSimLib.Layers.DetectorLayerBuilder;

export namespace BetaSimLib::Detectors {

class BaseDetectorConstruction final : public Geant4::G4VUserDetectorConstruction {
#pragma region Constructors/Destructors

public:
    explicit BaseDetectorConstruction(
        std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig> config
    )
        : config(std::move(config)) {
    }

    ~BaseDetectorConstruction() override = default;

    BaseDetectorConstruction(const BaseDetectorConstruction&) = delete;
    BaseDetectorConstruction& operator=(const BaseDetectorConstruction&) = delete;

    BaseDetectorConstruction(BaseDetectorConstruction&&) = delete;
    BaseDetectorConstruction& operator=(BaseDetectorConstruction&&) = delete;

#pragma endregion

#pragma region Methods

public:
    Geant4::G4VPhysicalVolume* Construct() override {
    if (!config) {
        Geant4::G4Exception(
            "BaseDetectorConstruction",
            "NullConfig",
            Geant4::G4ExceptionSeverity::FatalException,
            "BaseExperimentConfig is null."
        );

        return nullptr;
    }

    BetaSimLib::Materials::ExtendedMaterialService::Instance()
        .InitializeConfig(config.get());

    auto* worldMaterial = ResolveWorldMaterial(config->worldMaterial);

    const auto worldSize = ResolveWorldSize(config->worldSize);
    const auto halfWorldSize = worldSize / 2.0;

    auto* solidWorld = new Geant4::G4Box(
        "World_Solid",
        halfWorldSize,
        halfWorldSize,
        halfWorldSize
    );

    auto* logicWorld = new Geant4::G4LogicalVolume(
        solidWorld,
        worldMaterial,
        "World_Logic"
    );

    auto* physicalWorld = new Geant4::G4PVPlacement(
        nullptr,
        Geant4::G4ThreeVector(),
        logicWorld,
        "world",
        nullptr,
        false,
        0,
        true
    );

    BetaSimLib::Layers::SourceLayerBuilder sourceBuilder;
    sourceBuilder.Build(config->sourceLayer, logicWorld);

    BetaSimLib::Layers::DetectorLayerBuilder detectorBuilder;
    detectorBuilder.Build(config->detector, logicWorld);

    return physicalWorld;
}

    void ConstructSDandField() override {
        // Пока пусто.
        // Позже здесь можно назначать sensitive detectors и fields.
    }

private:
    void BuildSourceLayer(Geant4::G4LogicalVolume* worldLogic) const {
        if (!worldLogic) {
            return;
        }

        BetaSimLib::Layers::SourceLayerBuilder sourceBuilder;
        sourceBuilder.Build(config->sourceLayer, worldLogic);
    }

    void BuildDetectorLayer(Geant4::G4LogicalVolume* worldLogic) const {
        if (!worldLogic) {
            return;
        }

        BetaSimLib::Layers::DetectorLayerBuilder detectorBuilder;
        detectorBuilder.Build(config->detector, worldLogic);
    }

    Geant4::G4Material* ResolveWorldMaterial(const std::string& materialName) const {
        if (!materialName.empty()) {
            auto optExtMat =
                BetaSimLib::Materials::ExtendedMaterialService::Instance().Get(materialName);

            if (optExtMat.has_value() && optExtMat.value() != nullptr) {
                auto* g4Material = optExtMat.value()->GetG4Material();

                if (g4Material != nullptr) {
                    return g4Material;
                }
            }

            auto* nistMaterial =
                Geant4::G4NistManager::Instance()->FindOrBuildMaterial(materialName);

            if (nistMaterial != nullptr) {
                return nistMaterial;
            }
        }

        auto* fallback =
            Geant4::G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");

        if (fallback == nullptr) {
            Geant4::G4Exception(
                "BaseDetectorConstruction",
                "WorldMaterialNotFound",
                Geant4::G4ExceptionSeverity::FatalException,
                "Could not resolve world material."
            );

            return nullptr;
        }

        return fallback;
    }

    double ResolveWorldSize(double value) const {
        if (value > 0.0) {
            return value;
        }

        return 1.0 * Geant4::m;
    }

#pragma endregion

#pragma region Variables

private:
    std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig> config;

#pragma endregion
};

} // namespace BetaSimLib::Detectors