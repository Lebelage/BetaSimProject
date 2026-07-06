export module BetaSimLib.Models.Experiment;

import std;
import Geant4.Externals;

export namespace BetaSimLib::Models
{
    // ---------------------------------------------------------
    // БАЗОВЫЕ ПЕРЕЧИСЛЕНИЯ
    // ---------------------------------------------------------

    enum class ExpType {
        None,
        Stack
    };

    enum class SourceType {
        Gun,
        Decay
    };

    // ---------------------------------------------------------
    // СТРУКТУРЫ СЛОЕВ И БЛОКОВ
    // ---------------------------------------------------------

    struct LayerDef {
        std::string material;
        double thickness = 0.0;
    };

    struct DetectorConfig {
        double stackX = 0.0;
        double stackY = 0.0;
        std::vector<LayerDef> layers;
    };

    struct SourceLayerConfig {
        std::string type = "stack";
        double stackX = 0.0;
        double stackY = 0.0;
        std::vector<LayerDef> layers;
    };

    // ---------------------------------------------------------
    // СТРУКТУРЫ МАТЕРИАЛОВ
    // ---------------------------------------------------------

    struct MaterialBuildSpec {
        double density = 0.0;
        bool finalized = false;
        bool useAtoms = true;

        std::vector<std::pair<std::string, double>> mass;
    };

    struct MaterialBuildSpec_x {
        double x = 0.0;
        bool finalized = false;
    };

    // ---------------------------------------------------------
    // ИСТОЧНИКИ
    // ---------------------------------------------------------

    struct GunConfig {
        std::string particle = "e-";
        double energy = 0.0;
        Geant4::G4ThreeVector pos;
        Geant4::G4ThreeVector dir = Geant4::G4ThreeVector(0.0, 0.0, 1.0);
    };

    struct RadioIsotopeConfig {
        int z = 0;
        int a = 0;

        // Обычно 0, если изотоп в основном состоянии.
        double excitationEnergy = 0.0;

        // Обычно 0 для нейтрального атома.
        double ionCharge = 0.0;
    };

    // ---------------------------------------------------------
    // ГЛАВНЫЙ КОНФИГ
    // ---------------------------------------------------------

    struct BaseExperimentConfig {
        ExpType type = ExpType::None;

        // Мир
        std::string worldMaterial = "G4_Galactic";
        double worldSize = 0.0;

        // Геометрия
        DetectorConfig detector;
        SourceLayerConfig sourceLayer;

        // Материалы
        std::map<std::string, MaterialBuildSpec> matBuild;
        std::map<std::string, MaterialBuildSpec_x> matBuildX;

        // Источник
        SourceType sourceType = SourceType::Gun;
        GunConfig gun;
        RadioIsotopeConfig isotope;
    };
}