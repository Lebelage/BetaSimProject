//
// Created by Sonora on 18.06.2026.
//
export module BetaSimLib.Materials.ExtendedMaterialService;
import std;
import Geant4.Externals;
import BetaSimLib.Materials.ExtendedMaterial;
import BetaSimLib.Materials.MaterialConstants;
import BetaSimLib.Models.Experiment; // Подключаем модели для доступа к конфигу

export namespace BetaSimLib::Materials {
    using namespace BetaSimLib::Models; // Для BaseExperimentConfig

    class ExtendedMaterialService {
#pragma region Singleton

    public:
        static ExtendedMaterialService &Instance() {
            static ExtendedMaterialService instance;
            return instance;
        }

        ExtendedMaterialService(const ExtendedMaterialService &) = delete;
        ExtendedMaterialService &operator=(const ExtendedMaterialService &) = delete;
        ExtendedMaterialService(ExtendedMaterialService &&) = delete;
        ExtendedMaterialService &operator=(ExtendedMaterialService &&) = delete;

    private:
        ExtendedMaterialService() {
            nistManager = Geant4::G4NistManager::Instance();
        };

        ~ExtendedMaterialService() = default;
#pragma endregion

#pragma region Methods

    public:
        // ДОБАВЛЕНО: Метод для передачи конфигурации в сервис перед сборкой геометрии
        void InitializeConfig(const BaseExperimentConfig* config) {
            fCfg = config;
        }

        std::optional<ExtendedG4Material *> Get(const Geant4::G4String &name) {
            const std::string key = name;

            if (auto it = fCache.find(key); it != fCache.end())
                return it->second;

            // 1. СПЕЦИАЛЬНЫЙ СЛУЧАЙ: Радиоактивный Никель-63
            if (key == "Ni63_Source") {
                auto *isoNi63 = new Geant4::G4Isotope("Ni63", 28, 63, 62.9296 * Geant4::g / Geant4::mole);
                auto *elNi63 = new Geant4::G4Element("RadioactiveNi", "Ni", 1);
                elNi63->AddIsotope(isoNi63, 100.0 * Geant4::perCent);

                auto *ni63Mat = new Geant4::G4Material("Ni63_Material", 8.908 * Geant4::g / Geant4::cm3, 1);
                ni63Mat->AddElement(elNi63, 1.0);

                auto *mat = new ExtendedG4Material(ni63Mat, 0.0f, 0.0f, false);
                fCache[key] = mat;
                return mat;
            }

            // 2. NIST материалы
            if (name.rfind("G4_", 0) == 0) {
                auto *m = nistManager->FindOrBuildMaterial(name, true);
                if (!m)
                    Geant4::G4Exception("Materials", "NoNIST", Geant4::G4ExceptionSeverity::FatalException,
                                "Cannot build NIST material.");

                ExtendedG4Material *mat = new ExtendedG4Material(m, 0, 0, false);
                fCache[key] = mat;
                return mat;
            }

            // Проверяем, задан ли конфиг
            if (!fCfg) {
                Geant4::G4Exception("Materials", "NoConfig", Geant4::G4ExceptionSeverity::FatalException,
                            "ExtendedMaterialService configuration is not initialized.");
                return std::nullopt;
            }

            // 3. Материалы по массовым долям
            if (auto it = fCfg->matBuild.find(key); it != fCfg->matBuild.end()) {
                auto *m = BuildFromSpec(key, it->second);
                fCache[key] = m;
                return m;
            }

            // 4. Твердые растворы по x (AlGaN, InGaN)
            if (auto it = fCfg->matBuildX.find(key); it != fCfg->matBuildX.end()) {
                auto *m = BuildFromSpec(key, it->second);
                fCache[key] = m;
                return m;
            }

            Geant4::G4Exception("Materials", "UnknownMaterial", Geant4::G4ExceptionSeverity::FatalException,
                        ("Unknown material: " + key).c_str());

            return std::nullopt;
        };

    private:
        ExtendedG4Material *BuildFromSpec(const std::string &name,
                                          const MaterialBuildSpec_x &spec) {
            if (!spec.finalized) {
                Geant4::G4Exception("Materials", "NotFinalized", Geant4::G4ExceptionSeverity::FatalException,
                            ("Material not finalized: " + name).c_str());
            }

            double x = spec.x;
            double density = 0.0;
            std::vector<std::pair<std::string, double> > components;

            if (name.find("AlGaN") != std::string::npos || name.find("algan") != std::string::npos) {
                double m_total = x * MaterialsConstants::M_Al + (1.0 - x) * MaterialsConstants::M_Ga +
                                 MaterialsConstants::M_N;
                density = x * MaterialsConstants::DENS_AlN + (1.0 - x) * MaterialsConstants::DENS_GaN;

                components.push_back({"Al", (x * MaterialsConstants::M_Al) / m_total});
                components.push_back({"Ga", ((1.0 - x) * MaterialsConstants::M_Ga) / m_total});
                components.push_back({"N", MaterialsConstants::M_N / m_total});
            } else if (name.find("InGaN") != std::string::npos || name.find("ingan") != std::string::npos) {
                double m_total = x * MaterialsConstants::M_In + (1.0 - x) * MaterialsConstants::M_Ga +
                                 MaterialsConstants::M_N;
                density = x * MaterialsConstants::DENS_InN + (1.0 - x) * MaterialsConstants::DENS_GaN;

                components.push_back({"In", (x * MaterialsConstants::M_In) / m_total});
                components.push_back({"Ga", ((1.0 - x) * MaterialsConstants::M_Ga) / m_total});
                components.push_back({"N", MaterialsConstants::M_N / m_total});
            } else {
                Geant4::G4Exception("Materials", "UnknownAlloy", Geant4::G4ExceptionSeverity::FatalException,
                            ("Cannot build alloy from x for: " + name + ". Name must contain AlGaN or InGaN.").c_str());
            }

            // ИСПРАВЛЕНО: Префиксы Geant4::
            Geant4::G4Material *mat = new Geant4::G4Material(name, density * (Geant4::g / Geant4::cm3), components.size());

            for (const auto &[elName, fraction]: components) {
                // ИСПРАВЛЕНО: nistManager вместо fNist
                Geant4::G4Element *el = nistManager->FindOrBuildElement(elName, true);
                if (!el) {
                    Geant4::G4Exception("Materials", "NoElement", Geant4::G4ExceptionSeverity::FatalException,
                                ("Cannot build element: " + elName).c_str());
                }
                mat->AddElement(el, fraction);
            }

            return new ExtendedG4Material(mat, x, CalculateEg(name, x), true);
        }

        ExtendedG4Material *BuildFromSpec(const std::string &name,
                                          const MaterialBuildSpec &spec) {
            if (!spec.finalized) {
                Geant4::G4Exception("Materials", "NotFinalized", Geant4::G4ExceptionSeverity::FatalException,
                            ("Material not finalized: " + name).c_str());
            }
            if (spec.density <= 0) {
                Geant4::G4Exception("Materials", "BadDensity", Geant4::G4ExceptionSeverity::FatalException,
                            ("Bad density for material: " + name).c_str());
            }

            if (spec.useAtoms) {
                return nullptr; // atoms is not usable
            } else {
                Geant4::G4Material *mat = new Geant4::G4Material(name, spec.density, (int) spec.mass.size());
                for (const auto &m: spec.mass) {
                    // ИСПРАВЛЕНО: Обращение к std::pair через first и second, и nistManager
                    Geant4::G4Element *el = nistManager->FindOrBuildElement(m.first, true);
                    if (!el)
                        Geant4::G4Exception("Materials", "NoElement", Geant4::G4ExceptionSeverity::FatalException,
                                    "Cannot build element.");
                    mat->AddElement(el, m.second); // mass fraction
                }

                float Eg = 0.0f;
                if (name == "GaN")
                    Eg = MaterialsConstants::EG_GAN;

                return new ExtendedG4Material(mat, 0, Eg, false);
            };
        }

        float CalculateEg(std::string matName, float x) {
            if (x < 0.0f) x = 0.0f;
            if (x > 1.0f) x = 1.0f;

            if (matName.find("AlGaN") != std::string::npos || matName.find("algan") != std::string::npos) {
                return x * MaterialsConstants::EG_ALN +
                       (1.0f - x) * MaterialsConstants::EG_GAN -
                       MaterialsConstants::B_ALGAN * x * (1.0f - x);
            }
            else if (matName.find("InGaN") != std::string::npos || matName.find("ingan") != std::string::npos) {
                return x * MaterialsConstants::EG_INN +
                       (1.0f - x) * MaterialsConstants::EG_GAN -
                       MaterialsConstants::B_INGAN * x * (1.0f - x);
            }
            else if (matName.find("GaN") != std::string::npos || matName.find("gan") != std::string::npos) {
                return MaterialsConstants::EG_GAN;
            }

            std::cerr << "[ОШИБКА] Неизвестный материал для расчета Eg: " << matName << std::endl;
            return 0.0f;
        }
#pragma endregion

#pragma region Variables

    private:
        Geant4::G4NistManager* nistManager;
        const BaseExperimentConfig* fCfg = nullptr;
        std::unordered_map<std::string, ExtendedG4Material *> fCache;

#pragma endregion
    };
}