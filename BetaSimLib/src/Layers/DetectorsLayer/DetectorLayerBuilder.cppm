//
// Created by Sonora on 17.06.2026.
//
export module BetaSimLib.Layers.DetectorLayerBuilder;

import std;
import Geant4.Externals;

import BetaSimLib.Materials.ExtendedMaterialService;
import BetaSimLib.Models.Experiment;

export namespace BetaSimLib::Layers {

class BaseLayerBuilder {
protected:
    Geant4::G4Material* GetMaterial(const std::string& name) const {
        if (auto* existing = Geant4::G4Material::GetMaterial(name, false)) {
            return existing;
        }

        if (auto* nistMaterial =
                Geant4::G4NistManager::Instance()->FindOrBuildMaterial(name, false)) {
            return nistMaterial;
        }

        auto optExtMat =
            BetaSimLib::Materials::ExtendedMaterialService::Instance().Get(name);

        if (optExtMat.has_value() && optExtMat.value() != nullptr) {
            auto* material = optExtMat.value()->GetG4Material();

            if (material != nullptr) {
                return material;
            }
        }

        Geant4::G4Exception(
            "BaseLayerBuilder",
            "MatNotFound",
            Geant4::G4ExceptionSeverity::FatalException,
            ("Material not found: " + name).c_str()
        );

        return nullptr;
    }
};

// -----------------------------------------------------------------
// ДЕТЕКТОРНЫЕ СЛОИ
// Строятся вдоль -Z.
// Только эти logical volumes будут sensitive.
// -----------------------------------------------------------------
class DetectorLayerBuilder : public BaseLayerBuilder {
public:
    std::vector<Geant4::G4LogicalVolume*> Build(
        const BetaSimLib::Models::DetectorConfig& config,
        Geant4::G4LogicalVolume* motherVolume
    ) const {
        std::vector<Geant4::G4LogicalVolume*> detectorLogicalVolumes;

        if (config.layers.empty()) {
            return detectorLogicalVolumes;
        }

        if (!motherVolume) {
            Geant4::G4Exception(
                "DetectorLayerBuilder",
                "NullMotherVolume",
                Geant4::G4ExceptionSeverity::FatalException,
                "Mother volume is null."
            );

            return detectorLogicalVolumes;
        }

        const double halfX = config.stackX / 2.0;
        const double halfY = config.stackY / 2.0;

        double currentZ = 0.0;

        int layerIndex = 0;

        for (const auto& layer : config.layers) {
            const double halfZ = layer.thickness / 2.0;

            currentZ -= halfZ;

            const auto solidName =
                "DetLayer_Solid_" + std::to_string(layerIndex);

            auto* solid = new Geant4::G4Box(
                solidName,
                halfX,
                halfY,
                halfZ
            );

            const auto logicName =
                "DetLayer_Logic_" + std::to_string(layerIndex);

            auto* logic = new Geant4::G4LogicalVolume(
                solid,
                GetMaterial(layer.material),
                logicName
            );

            detectorLogicalVolumes.push_back(logic);

            const auto physName =
                "DetLayer_Phys_" + std::to_string(layerIndex);

            new Geant4::G4PVPlacement(
                nullptr,
                Geant4::G4ThreeVector(0.0, 0.0, currentZ),
                logic,
                physName,
                motherVolume,
                false,
                layerIndex,
                true
            );

            currentZ -= halfZ;
            ++layerIndex;
        }

        return detectorLogicalVolumes;
    }
};

// -----------------------------------------------------------------
// SOURCE СЛОИ
// Строятся вдоль +Z.
// НЕ sensitive.
// -----------------------------------------------------------------
class SourceLayerBuilder : public BaseLayerBuilder {
public:
    void Build(
        const BetaSimLib::Models::SourceLayerConfig& config,
        Geant4::G4LogicalVolume* motherVolume
    ) const {
        if (config.layers.empty()) {
            return;
        }

        if (!motherVolume) {
            Geant4::G4Exception(
                "SourceLayerBuilder",
                "NullMotherVolume",
                Geant4::G4ExceptionSeverity::FatalException,
                "Mother volume is null."
            );

            return;
        }

        if (config.type == "wrapped_stack") {
            BuildWrappedStack(config, motherVolume);
        } else {
            BuildNormalStack(config, motherVolume);
        }
    }

private:
    void BuildNormalStack(
        const BetaSimLib::Models::SourceLayerConfig& config,
        Geant4::G4LogicalVolume* motherVolume
    ) const {
        const double halfX = config.stackX / 2.0;
        const double halfY = config.stackY / 2.0;

        double currentZ = 0.0;

        int layerIndex = 0;

        for (const auto& layer : config.layers) {
            const double halfZ = layer.thickness / 2.0;

            currentZ += halfZ;

            auto* solid = new Geant4::G4Box(
                "SrcLayer_Solid_" + std::to_string(layerIndex),
                halfX,
                halfY,
                halfZ
            );

            auto* logic = new Geant4::G4LogicalVolume(
                solid,
                GetMaterial(layer.material),
                "SrcLayer_Logic_" + std::to_string(layerIndex)
            );

            new Geant4::G4PVPlacement(
                nullptr,
                Geant4::G4ThreeVector(0.0, 0.0, currentZ),
                logic,
                "SrcLayer_Phys_" + std::to_string(layerIndex),
                motherVolume,
                false,
                layerIndex,
                true
            );

            currentZ += halfZ;
            ++layerIndex;
        }
    }

    void BuildWrappedStack(
        const BetaSimLib::Models::SourceLayerConfig& config,
        Geant4::G4LogicalVolume* motherVolume
    ) const {
        // TODO: позже реализуешь wrapped source.
        BuildNormalStack(config, motherVolume);
    }
};

} // namespace BetaSimLib::Layers