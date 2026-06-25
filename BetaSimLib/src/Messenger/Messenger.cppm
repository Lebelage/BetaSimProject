//
// Created by Sonora on 17.06.2026.
//
export module BetaSimLib.Messenger;

import std;
import Geant4.Externals;
import BetaSimLib.Commands.CommandBuilder;
import BetaSimLib.Commands.CommandManager;
import BetaSimLib.Models.Experiment;

export namespace BetaSimLib::Messenger
{
  using namespace BetaSimLib::Commands;

  // Перечисление для отслеживания текущего контекста парсинга макроса
  enum class ParseState {
    Global,
    InDetector,
    InSource
  };

  class BaseExperimentMessenger : public Geant4::G4UImessenger {
#pragma region Constructor/Destructor
  public:
    BaseExperimentMessenger()
    {
      fCfg = std::make_unique<BetaSimLib::Models::BaseExperimentConfig>();
      commandManager = std::make_unique<CommandManager>();
      G4CommandBuilder builder(this);

      fDir = builder.Directory("/exp/", "Experiment configuration");

      // --------------------------------------------------------
      // ГЛОБАЛЬНЫЕ КОМАНДЫ
      // --------------------------------------------------------
      fReset = builder.Command("/exp/reset");
      fType = builder.String("/exp/type");
      applyCommand = builder.Command("/exp/apply");

      // World
      fWorldMat = builder.String("/exp/world/material");
      fWorldSize = builder.DoubleUnit("/exp/world/size", "Length");

      // Materials
      fMatCreate = builder.Params("/exp/material/create", Param("name", 's'),
                                  Param("density", 'd'), Param("densUnit", 's'));
      fMatCreateX = builder.Params("/exp/material/createx", Param("mat", 's'), Param("x", 'd'));
      fMatAddMass = builder.Params("/exp/material/addMassFraction", Param("mat", 's'),
                                   Param("el", 's'), Param("fraction", 'd'));
      fMatFinalize = builder.Params("/exp/material/finalize", Param("mat", 's'));

      // Базовый источник (Физика)
      fSourceType = builder.Candidates("/exp/source/type", "gun decay");
      fGunParticle = builder.String("/exp/source/gun/particle");
      fGunEnergy = builder.DoubleUnit("/exp/source/gun/energy", "Energy");
      fGunPos = builder.Vec3Unit("/exp/source/gun/pos", "Length");
      fGunDir = builder.Params("/exp/source/gun/dir", Param("dx", 'd'),
                               Param("dy", 'd'), Param("dz", 'd'));

      // --------------------------------------------------------
      // КОМАНДЫ КОНТЕКСТА (БЛОКИ)
      // --------------------------------------------------------
      // Детектор
      fDetStart = builder.Command("/exp/DetectorLayer/start");
      fDetEnd = builder.Command("/exp/DetectorLayer/end");

      // Сложный источник
      fSrcLayerStart = builder.Command("/exp/SourceLayer/start");
      fSrcLayerEnd = builder.Command("/exp/SourceLayer/end");
      fSrcLayerType = builder.String("/exp/SourceLayerType/");

      // --------------------------------------------------------
      // УНИВЕРСАЛЬНЫЕ КОМАНДЫ (Зависят от активного блока)
      // --------------------------------------------------------
      fStackXY = builder.Params("/exp/stack/xy", Param("x", 'd'), Param("xUnit", 's'),
                                                 Param("y", 'd'), Param("yUnit", 's'));

      fLayerAdd = builder.Params("/exp/layer/add", Param("mat", 's'),
                                                   Param("th", 'd'), Param("unit", 's'));

      fLayersClear = builder.Command("/exp/layers/clear");
    };

    ~BaseExperimentMessenger() override { Release(); };

    BaseExperimentMessenger(const BaseExperimentMessenger &) = delete;
    BaseExperimentMessenger &operator=(const BaseExperimentMessenger &) = delete;
    BaseExperimentMessenger(BaseExperimentMessenger &&) = delete;
    BaseExperimentMessenger &operator=(BaseExperimentMessenger &&) = delete;
#pragma endregion

#pragma region Methods
  private:
    void SetNewValue(Geant4::G4UIcommand *cmd, Geant4::G4String value) override {

      // --------------------------------------------------------
      // БАЗОВЫЕ УПРАВЛЯЮЩИЕ КОМАНДЫ
      // --------------------------------------------------------
      if (cmd == applyCommand) {
        if (fState != ParseState::Global) {
          Geant4::G4Exception("Messenger", "ApplyError", Geant4::G4ExceptionSeverity::JustWarning,
                              "Calling /exp/apply while a block is still open. Proceeding anyway, but data might be incomplete.");
        }
        //commandManager->ApplyCommand(std::move(fCfg));
        return;
      }

      if (cmd == fReset) {
        fCfg = std::make_unique<BetaSimLib::Models::BaseExperimentConfig>();
        fState = ParseState::Global;
        return;
      }

      if (cmd == fType) {
        GetConfigInstance()->type = ParseType(value);
        return;
      }

      // --------------------------------------------------------
      // УПРАВЛЕНИЕ БЛОКАМИ (STATE MACHINE)
      // --------------------------------------------------------
      if (cmd == fDetStart) {
        if (fState != ParseState::Global) {
          Geant4::G4Exception("Messenger", "StateError", Geant4::G4ExceptionSeverity::FatalException, "Cannot start DetectorLayer inside another block.");
        }
        fState = ParseState::InDetector;
        fTempDetector = BetaSimLib::Models::DetectorConfig(); // Очищаем временный буфер
        return;
      }

      if (cmd == fDetEnd) {
        if (fState != ParseState::InDetector) {
          Geant4::G4Exception("Messenger", "StateError", Geant4::G4ExceptionSeverity::FatalException, "Mismatched /exp/DetectorLayer/end.");
        }
        fState = ParseState::Global;
        GetConfigInstance()->detector = fTempDetector; // Сохраняем в основной конфиг
        return;
      }

      if (cmd == fSrcLayerStart) {
        if (fState != ParseState::Global) {
          Geant4::G4Exception("Messenger", "StateError", Geant4::G4ExceptionSeverity::FatalException, "Cannot start SourceLayer inside another block.");
        }
        fState = ParseState::InSource;
        fTempSourceLayer = BetaSimLib::Models::SourceLayerConfig();
        return;
      }

      if (cmd == fSrcLayerEnd) {
        if (fState != ParseState::InSource) {
          Geant4::G4Exception("Messenger", "StateError", Geant4::G4ExceptionSeverity::FatalException, "Mismatched /exp/SourceLayer/end.");
        }
        fState = ParseState::Global;
        GetConfigInstance()->sourceLayer = fTempSourceLayer;
        return;
      }

      // --------------------------------------------------------
      // УНИВЕРСАЛЬНЫЕ КОМАНДЫ (Слои, Стек)
      // --------------------------------------------------------
      if (cmd == fStackXY) {
        Geant4::G4Tokenizer tok(value);
        double x = std::stod(tok()) * Geant4::G4UIcommand::ValueOf(tok());
        double y = std::stod(tok()) * Geant4::G4UIcommand::ValueOf(tok());

        if (fState == ParseState::InDetector) {
          fTempDetector.stackX = x;
          fTempDetector.stackY = y;
        } else if (fState == ParseState::InSource) {
          fTempSourceLayer.stackX = x;
          fTempSourceLayer.stackY = y;
        } else {
          Geant4::G4Exception("Messenger", "ContextWarning", Geant4::G4ExceptionSeverity::JustWarning, "/exp/stack/xy used globally. Ignoring.");
        }
        return;
      }

      if (cmd == fLayerAdd) {
        Geant4::G4Tokenizer tok(value);
        auto mat = tok();
        double th = std::stod(tok()) * Geant4::G4UIcommand::ValueOf(tok());

        if (fState == ParseState::InDetector) {
          fTempDetector.layers.push_back({mat, th});
        } else if (fState == ParseState::InSource) {
          fTempSourceLayer.layers.push_back({mat, th});
        } else {
          Geant4::G4Exception("Messenger", "ContextWarning", Geant4::G4ExceptionSeverity::JustWarning, "/exp/layer/add used globally. Ignoring.");
        }
        return;
      }

      if (cmd == fLayersClear) {
        if (fState == ParseState::InDetector) {
          fTempDetector.layers.clear();
        } else if (fState == ParseState::InSource) {
          fTempSourceLayer.layers.clear();
        }
        return;
      }

      if (cmd == fSrcLayerType && fState == ParseState::InSource) {
        fTempSourceLayer.type = value;
        return;
      }

      // --------------------------------------------------------
      // МИР (WORLD)
      // --------------------------------------------------------
      if (cmd == fWorldMat) {
        GetConfigInstance()->worldMaterial = value;
        return;
      }

      if (cmd == fWorldSize) {
        GetConfigInstance()->worldSize = fWorldSize->GetNewDoubleValue(value);
        return;
      }

      // --------------------------------------------------------
      // МАТЕРИАЛЫ
      // --------------------------------------------------------
      if (cmd == fMatCreate) {
        Geant4::G4Tokenizer tok(value);
        auto name = tok();
        auto dens = tok();
        auto unit = tok();

        BetaSimLib::Models::MaterialBuildSpec spec;
        spec.density = std::stod(dens) * Geant4::G4UIcommand::ValueOf(unit);
        spec.finalized = false;
        spec.useAtoms = true;

        GetConfigInstance()->matBuild[name] = spec;
        return;
      }

      if (cmd == fMatCreateX) {
        Geant4::G4Tokenizer tok(value);
        Geant4::G4String matName = tok();
        double x = std::stod(tok());

        BetaSimLib::Models::MaterialBuildSpec_x spec;
        spec.x = x;
        spec.finalized = true;

        GetConfigInstance()->matBuildX[matName] = spec;
        return;
      }

      if (cmd == fMatAddMass) {
        Geant4::G4Tokenizer tok(value);
        auto mat = tok();
        auto el = tok();
        auto fr = tok();

        GetConfigInstance()->matBuild[mat].useAtoms = false;
        GetConfigInstance()->matBuild[mat].mass.push_back({el, std::stod(fr)});
        return;
      }

      if (cmd == fMatFinalize) {
        Geant4::G4Tokenizer tok(value);
        auto mat = tok();
        GetConfigInstance()->matBuild[mat].finalized = true;
        return;
      }

      // --------------------------------------------------------
      // ИСТОЧНИК (ФИЗИКА)
      // --------------------------------------------------------
      if (cmd == fSourceType) {
        GetConfigInstance()->sourceType = (value == "gun")
                              ? BetaSimLib::Models::SourceType::Gun
                              : BetaSimLib::Models::SourceType::Decay;
        return;
      }

      if (cmd == fGunParticle) {
        GetConfigInstance()->gun.particle = value;
        return;
      }

      if (cmd == fGunEnergy) {
        GetConfigInstance()->gun.energy = fGunEnergy->GetNewDoubleValue(value);
        return;
      }

      if (cmd == fGunPos) {
        GetConfigInstance()->gun.pos = fGunPos->GetNew3VectorValue(value);
        return;
      }

      if (cmd == fGunDir) {
        Geant4::G4Tokenizer tok(value);
        auto dx = tok();
        auto dy = tok();
        auto dz = tok();

        GetConfigInstance()->gun.dir =
            Geant4::G4ThreeVector(std::stod(dx), std::stod(dy), std::stod(dz)).unit();
        return;
      }
    }

    void Release() {
      // Очистка команд контекста
      delete fDetStart;
      delete fDetEnd;
      delete fSrcLayerStart;
      delete fSrcLayerEnd;
      delete fSrcLayerType;

      // Очистка команд физики и источника
      delete fGunDir;
      delete fGunPos;
      delete fGunEnergy;
      delete fGunParticle;
      delete fSourceType;

      // Очистка команд материалов
      delete fMatFinalize;
      delete fMatCreateX;
      delete fMatAddMass;
      delete fMatCreate;

      // Очистка универсальных команд
      delete fLayerAdd;
      delete fLayersClear;
      delete fStackXY;

      // Очистка команд мира и общих
      delete fWorldSize;
      delete fWorldMat;
      delete fType;
      delete fReset;
      delete applyCommand;

      delete fDir;
    }

  private:
    static BetaSimLib::Models::ExpType ParseType(const Geant4::G4String &s) {
      if (s == "stack")
        return BetaSimLib::Models::ExpType::Stack;
      return BetaSimLib::Models::ExpType::None;
    }
#pragma endregion

#pragma region Properties
  private:
    BetaSimLib::Models::BaseExperimentConfig* GetConfigInstance() {
      if (fCfg == nullptr) {
        fCfg = std::make_unique<BetaSimLib::Models::BaseExperimentConfig>();
      }
      return fCfg.get();
    };
#pragma endregion

#pragma region Fields
  private:
    // Текущее состояние парсера
    ParseState fState = ParseState::Global;

    // Временные структуры для блоков (требуется реализация в моделях)
    BetaSimLib::Models::DetectorConfig fTempDetector;
    BetaSimLib::Models::SourceLayerConfig fTempSourceLayer;

    // Конфигурация и менеджеры
    std::unique_ptr<BetaSimLib::Models::BaseExperimentConfig> fCfg;
    std::unique_ptr<CommandManager> commandManager;

    Geant4::G4UIdirectory *fDir = nullptr;

    // Общие
    Geant4::G4UIcommand *fReset = nullptr;
    Geant4::G4UIcmdWithAString *fType = nullptr;
    Geant4::G4UIcommand *applyCommand = nullptr;

    // Мир
    Geant4::G4UIcmdWithAString *fWorldMat = nullptr;
    Geant4::G4UIcmdWithADoubleAndUnit *fWorldSize = nullptr;

    // Контекстные команды для слоев и стека
    Geant4::G4UIcommand *fStackXY = nullptr;
    Geant4::G4UIcommand *fLayersClear = nullptr;
    Geant4::G4UIcommand *fLayerAdd = nullptr;

    // Команды блоков
    Geant4::G4UIcommand *fDetStart = nullptr;
    Geant4::G4UIcommand *fDetEnd = nullptr;

    Geant4::G4UIcommand *fSrcLayerStart = nullptr;
    Geant4::G4UIcommand *fSrcLayerEnd = nullptr;
    Geant4::G4UIcmdWithAString *fSrcLayerType = nullptr;

    // Материалы
    Geant4::G4UIcommand *fMatCreate = nullptr;
    Geant4::G4UIcommand *fMatCreateX = nullptr;
    Geant4::G4UIcommand *fMatAddMass = nullptr;
    Geant4::G4UIcommand *fMatFinalize = nullptr;

    // Источник (Физика)
    Geant4::G4UIcmdWithAString *fSourceType = nullptr;
    Geant4::G4UIcmdWithAString *fGunParticle = nullptr;
    Geant4::G4UIcmdWithADoubleAndUnit *fGunEnergy = nullptr;
    Geant4::G4UIcmdWith3VectorAndUnit *fGunPos = nullptr;
    Geant4::G4UIcommand *fGunDir = nullptr;
#pragma endregion
  };
}