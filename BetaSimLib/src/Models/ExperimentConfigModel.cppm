
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

    // Описание одного слоя
    struct LayerDef {
        std::string material;
        double thickness = 0.0;
    };

    // Конфигурация для блока детектора (/exp/DetectorLayer/...)
    struct DetectorConfig {
        double stackX = 0.0;
        double stackY = 0.0;
        std::vector<LayerDef> layers;
    };

    // Конфигурация для блока источника (/exp/SourceLayer/...)
    struct SourceLayerConfig {
        std::string type; // e.g. "stack", "wrapped", "complex"
        double stackX = 0.0;
        double stackY = 0.0;
        std::vector<LayerDef> layers;
    };

    // ---------------------------------------------------------
    // СТРУКТУРЫ МАТЕРИАЛОВ И ФИЗИКИ
    // ---------------------------------------------------------

    // Спецификация сборки сложного материала по массовым долям
    struct MaterialBuildSpec {
        double density = 0.0;
        bool finalized = false;
        bool useAtoms = true;
        // Пара <Имя элемента, Доля>
        std::vector<std::pair<std::string, double>> mass;
    };

    // Спецификация для твердых растворов (например, Al_x Ga_{1-x} N)
    struct MaterialBuildSpec_x {
        double x = 0.0;
        bool finalized = false;
    };

    // Настройки Particle Gun
    struct GunConfig {
        std::string particle = "e-";
        double energy = 0.0;
        Geant4::G4ThreeVector pos;
        Geant4::G4ThreeVector dir;
    };

    // ---------------------------------------------------------
    // ГЛАВНЫЙ КОНФИГ
    // ---------------------------------------------------------

    // Единый объект конфигурации эксперимента, который передается в ApplyCommand
    struct BaseExperimentConfig {
        ExpType type = ExpType::None;

        // Мир (World)
        std::string worldMaterial = "G4_Galactic";
        double worldSize = 0.0;

        // Контекстные блоки
        DetectorConfig detector;
        SourceLayerConfig sourceLayer;

        // Материалы (Словари, где ключ - название материала)
        std::map<std::string, MaterialBuildSpec> matBuild;
        std::map<std::string, MaterialBuildSpec_x> matBuildX;

        // Источник (Физика)
        SourceType sourceType = SourceType::Gun;
        GunConfig gun;
    };
}