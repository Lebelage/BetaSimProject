//
// Created by Sonora on 17.06.2026.
//
export module BetaSimLib.Layers.DetectorLayerBuilder;
import std;
import Geant4.Externals;
import BetaSimLib.Materials.ExtendedMaterialService;
import BetaSimLib.Models.Experiment;
export namespace BetaSimLib::Layers {
    // -----------------------------------------------------------------
    // БАЗОВЫЙ КЛАСС ДЛЯ ОБЩИХ МЕТОДОВ
    // -----------------------------------------------------------------
    class BaseLayerBuilder {
    protected:
        // Безопасное извлечение G4Material из вашего сервиса
        Geant4::G4Material* GetMaterial(const std::string& name) const {
            auto optExtMat = BetaSimLib::Materials::ExtendedMaterialService::Instance().Get(name);
            if (!optExtMat.has_value() || optExtMat.value() == nullptr) {
                Geant4::G4Exception("BaseLayerBuilder", "MatNotFound", Geant4::G4ExceptionSeverity::FatalException,
                                    ("Material not found in service: " + name).c_str());
            }

            return optExtMat.value()->GetG4Material();
        }
    };

    // -----------------------------------------------------------------
    // БИЛДЕР ДЕТЕКТОРА (Строится вдоль оси -Z)
    // -----------------------------------------------------------------
    class DetectorLayerBuilder : public BaseLayerBuilder {
    public:
        void Build(const BetaSimLib::Models::DetectorConfig& config, Geant4::G4LogicalVolume* motherVolume) const
        {
            if (config.layers.empty()) return;

            double currentZ = 0.0; // Старт из 0 в минус
            double halfX = config.stackX / 2.0;
            double halfY = config.stackY / 2.0;

            int layerIndex = 0;
            for (const auto& layer : config.layers) {
                double halfZ = layer.thickness / 2.0;

                currentZ -= halfZ;

                std::string solidName = "DetLayer_Solid_" + std::to_string(layerIndex);
                auto* solid = new Geant4::G4Box(solidName, halfX, halfY, halfZ);

                // Логика
                std::string logicName = "DetLayer_Logic_" + std::to_string(layerIndex);
                auto* logic = new Geant4::G4LogicalVolume(solid, GetMaterial(layer.material), logicName);

                // Физика (Размещение)
                std::string physName = "DetLayer_Phys_" + std::to_string(layerIndex);
                new Geant4::G4PVPlacement(nullptr,
                                          Geant4::G4ThreeVector(0, 0, currentZ),
                                          logic, physName, motherVolume, false, layerIndex, true);
                
                currentZ -= halfZ;
                layerIndex++;
            }
        }
    };

    // -----------------------------------------------------------------
    // БИЛДЕР ИСТОЧНИКА (Строится вдоль оси +Z)
    // -----------------------------------------------------------------
    class SourceLayerBuilder : public BaseLayerBuilder {
    public:
        void Build(const BetaSimLib::Models::SourceLayerConfig& config, Geant4::G4LogicalVolume* motherVolume) const
        {
            if (config.layers.empty()) return;

            if (config.type == "wrapped_stack") {
                BuildWrappedStack(config, motherVolume);
            } else {
                BuildNormalStack(config, motherVolume);
            }
        }

    private:
        // Обычный стек: строится от 0 в сторону +Z
        void BuildNormalStack(const BetaSimLib::Models::SourceLayerConfig& config, Geant4::G4LogicalVolume* motherVolume) const
        {
            double currentZ = 0.0;
            double halfX = config.stackX / 2.0;
            double halfY = config.stackY / 2.0;

            int layerIndex = 0;
            for (const auto& layer : config.layers) {
                double halfZ = layer.thickness / 2.0;

                currentZ += halfZ; // Сдвиг в центр слоя по оси +Z

                auto* solid = new Geant4::G4Box("SrcLayer_Solid_" + std::to_string(layerIndex),
                                                halfX, halfY, halfZ);
                auto* logic = new Geant4::G4LogicalVolume(solid, GetMaterial(layer.material),
                                                          "SrcLayer_Logic_" + std::to_string(layerIndex));

                new Geant4::G4PVPlacement(nullptr, Geant4::G4ThreeVector(0, 0, currentZ),
                                          logic, "SrcLayer_Phys_" + std::to_string(layerIndex),
                                          motherVolume, false, layerIndex, true);

                currentZ += halfZ; // На верхнюю границу
                layerIndex++;
            }
        }

        void BuildWrappedStack(const BetaSimLib::Models::SourceLayerConfig& config, Geant4::G4LogicalVolume* motherVolume) const
        {
            // double innerZTotal = 0.0;
            // for (const auto& layer : config.layers) {
            //     innerZTotal += layer.thickness;
            // }
            
            // double t = config.wrapperThickness;
            // double outerX = config.stackX + 2.0 * t;
            // double outerY = config.stackY + 2.0 * t;
            // double outerZ = innerZTotal + 2.0 * t;

            // // 3. Создаем оболочку
            // auto* wrapperSolid = new Geant4::G4Box("SrcWrapper_Solid", outerX / 2.0, outerY / 2.0, outerZ / 2.0);
            // auto* wrapperLogic = new Geant4::G4LogicalVolume(wrapperSolid,
            //                                                  GetMaterial(config.wrapperMaterial),
            //                                                  "SrcWrapper_Logic");

            // // Оболочка размещается в Мире. Нижняя стенка должна быть в Z=0,
            // // значит центр оболочки находится в Z = outerZ / 2.0
            // new Geant4::G4PVPlacement(nullptr, Geant4::G4ThreeVector(0, 0, outerZ / 2.0),
            //                           wrapperLogic, "SrcWrapper_Phys",
            //                           motherVolume, false, 0, true);

            // // 4. Размещаем внутренние слои ВНУТРИ оболочки
            // // Внутренняя полость начинается сразу после нижней стенки (толщиной t).
            // // В локальных координатах оболочки (от -outerZ/2 до +outerZ/2):
            // // Старт = -outerZ/2 + t = -innerZTotal / 2.0

            // double localCurrentZ = -innerZTotal / 2.0;
            // double halfX = config.stackX / 2.0;
            // double halfY = config.stackY / 2.0;

            // int layerIndex = 0;
            // for (const auto& layer : config.layers) {
            //     double halfZ = layer.thickness / 2.0;

            //     localCurrentZ += halfZ; // Центр внутреннего слоя

            //     auto* solid = new Geant4::G4Box("SrcInner_Solid_" + std::to_string(layerIndex),
            //                                     halfX, halfY, halfZ);
            //     auto* logic = new Geant4::G4LogicalVolume(solid, GetMaterial(layer.material),
            //                                               "SrcInner_Logic_" + std::to_string(layerIndex));

            //     // ВАЖНО: Размещаем в wrapperLogic!
            //     new Geant4::G4PVPlacement(nullptr, Geant4::G4ThreeVector(0, 0, localCurrentZ),
            //                               logic, "SrcInner_Phys_" + std::to_string(layerIndex),
            //                               wrapperLogic, false, layerIndex, true);

            //     localCurrentZ += halfZ; // Переход на верхнюю границу
            //     layerIndex++;
            // }
        }
    };
}