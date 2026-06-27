export module BetaSimLib.Runner;

import std;
import Geant4.Externals;

import BetaSimLib.Detectors.DetectorManager;
import BetaSimLib.Detectors.StartDetector;

import BetaSimLib.EventDispatcher;
import BetaSimLib.Models.Experiment;
import BetaSimLib.Messenger;

import BetaSimLib.Actions.RunAction;
import BetaSimLib.Generators.GeneratorManager;
import BetaSimLib.Runtime.ExperimentState;

export namespace BetaSimLib::Runner {

class UserActionInitialization : public Geant4::G4VUserActionInitialization {
#pragma region Constructors/Destructors

public:
  UserActionInitialization(
      std::shared_ptr<BetaSimLib::Runtime::ExperimentState> state,
      std::shared_ptr<BetaSimLib::Generators::GeneratorManager>
          generatorManager)
      : state(std::move(state)), generatorManager(std::move(generatorManager)) {
  }

  ~UserActionInitialization() override = default;

  UserActionInitialization(const UserActionInitialization &) = delete;
  UserActionInitialization &
  operator=(const UserActionInitialization &) = delete;

  UserActionInitialization(UserActionInitialization &&) = delete;
  UserActionInitialization &operator=(UserActionInitialization &&) = delete;

#pragma endregion

#pragma region Methods

public:
  void BuildForMaster() const override {
    SetUserAction(new BetaSimLib::Actions::RunAction());
  }

  void Build() const override {
    SetUserAction(new BetaSimLib::Actions::RunAction());

    // Каждый worker thread получает свой PrimaryGeneratorAction.
    SetUserAction(generatorManager->CreateGeneratorForWorker());

    // Потом сюда можно добавить:
    // SetUserAction(new BetaSimLib::Actions::EventAction(state));
    // SetUserAction(new BetaSimLib::Actions::SteppingAction(state));
  }

#pragma endregion

#pragma region Variables

private:
  std::shared_ptr<BetaSimLib::Runtime::ExperimentState> state;
  std::shared_ptr<BetaSimLib::Generators::GeneratorManager> generatorManager;

#pragma endregion
};

class Runner {
#pragma region Singleton

public:
  static Runner &GetInstance() {
    static Runner instance;
    return instance;
  }

  Runner(const Runner &) = delete;
  Runner &operator=(const Runner &) = delete;

  Runner(Runner &&) = delete;
  Runner &operator=(Runner &&) = delete;

#pragma endregion

#pragma region Constructors/Destructor

private:
  Runner() = default;
  ~Runner() = default;

#pragma endregion

#pragma region Public Methods

public:
  void Initialize(int argc, char **argv) {
    InitializeRunManager(argc, argv);
    SubscribeEvents();
  }

  void InitializeUI(int argc, char **argv) {
    uiManager = Geant4::G4UImanager::GetUIpointer();

    const bool isInteractive = IsInteractive(argc);

    if (!isInteractive) {
      ExecuteBatchMacro(argv[1]);
      return;
    }

    visManager = std::make_unique<Geant4::G4VisExecutive>();
    visManager->Initialize();

    uiManager->ApplyCommand("/control/macroPath AppConfigs");

    // init.mac должен только открыть viewer.
    // Не вызывай там /vis/drawVolume до загрузки config,
    // если StartDetector не гарантирует валидный world.
    uiManager->ApplyCommand("/control/execute init.mac");

    ui->SessionStart();
  }

#pragma endregion

#pragma region Initialization

private:
  void InitializeRunManager(int argc, char **argv) {
    const bool isInteractive = IsInteractive(argc);

    if (isInteractive) {
      ui = std::make_unique<Geant4::G4UIExecutive>(argc, argv);
    }

    runManager = std::make_unique<Geant4::G4MTRunManager>();

    expMessenger =
        std::make_unique<BetaSimLib::Messenger::BaseExperimentMessenger>();

    experimentState = std::make_shared<BetaSimLib::Runtime::ExperimentState>();

    config = std::make_shared<const BetaSimLib::Models::BaseExperimentConfig>();

    experimentState->SetConfig(config);

    detManager = std::make_unique<BetaSimLib::Detectors::DetectorManager>();

    detManager->SetDetector(
        std::make_unique<BetaSimLib::Detectors::StartDetector>());

    genManager = std::make_shared<BetaSimLib::Generators::GeneratorManager>(
        experimentState);

    const auto threadCount = ResolveThreadCount(argc, argv);
    runManager->SetNumberOfThreads(threadCount);

    runManager->SetUserInitialization(detManager->GetCurrentDetectorPointer());

    runManager->SetUserInitialization(InitializePhysics());

    runManager->SetUserInitialization(
        new UserActionInitialization(experimentState, genManager));

    runManager->Initialize();
  }

  Geant4::FTFP_BERT *InitializePhysics() {
    auto *physics = new Geant4::FTFP_BERT();

    physics->ReplacePhysics(new Geant4::G4EmStandardPhysics_option4());

    physics->RegisterPhysics(new Geant4::G4StepLimiterPhysics());

    // FTFP_BERT уже содержит обычный DecayPhysics.
    // Не добавляй G4DecayPhysics второй раз, иначе будет warning Duplicate type
    // for Decay.
    //
    // physics->RegisterPhysics(new Geant4::G4DecayPhysics());

    physics->RegisterPhysics(new Geant4::G4RadioactiveDecayPhysics());

    return physics;
  }

  void SubscribeEvents() {
    BetaSimLib::EventDispatcher::EventDispatcher::
        GetExperimentConfigReadyEvent()
            .Add([this](std::shared_ptr<
                        const BetaSimLib::Models::BaseExperimentConfig>
                            cfg) { OnExperimentUpdated(std::move(cfg)); });
  }

#pragma endregion

#pragma region Event Handlers

private:
  void OnExperimentUpdated(
      std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig>
          newConfig) {
    uiManager->ApplyCommand("/vis/disable");
    if (!newConfig) {
      Geant4::G4Exception("Runner", "NullExperimentConfig",
                          Geant4::G4ExceptionSeverity::JustWarning,
                          "Received null experiment config.");

      return;
    }

    config = std::move(newConfig);

    experimentState->SetConfig(config);

    detManager->ApplyConfig(config);
    genManager->ApplyConfig(config);

    if (runManager) {
      runManager->GeometryHasBeenModified();
      runManager->ReinitializeGeometry();
    }

    runManager->BeamOn(0);
    
    uiManager->ApplyCommand("/vis/scene/clear");
    uiManager->ApplyCommand("/vis/scene/create");
    uiManager->ApplyCommand("/vis/scene/add/volume world");
    uiManager->ApplyCommand("/vis/sceneHandler/attach");

    uiManager->ApplyCommand("/vis/enable");

    uiManager->ApplyCommand("/vis/viewer/refresh");
  }

#pragma endregion

#pragma region Helpers

private:
  static bool IsInteractive(int argc) { return argc == 1; }

  int ResolveThreadCount(int argc, char **argv) const {
    const bool isInteractive = IsInteractive(argc);

    // Режимы:
    //
    // ./Application
    //      Qt UI, 1 thread
    //
    // ./Application batch.mac
    //      batch, 8 threads
    //
    // ./Application batch.mac 4
    //      batch, 4 threads
    //
    // ./Application any.mac 1
    //      batch, 1 thread

    if (argc >= 3) {
      try {
        const int requestedThreads = std::stoi(argv[2]);
        return std::max(1, requestedThreads);
      } catch (...) {
        Geant4::G4Exception("Runner", "InvalidThreadArgument",
                            Geant4::G4ExceptionSeverity::JustWarning,
                            "Invalid thread count argument. Falling back to "
                            "default thread count.");
      }
    }

    // Для Qt/OpenGL interactive безопаснее 1 поток.
    // Для batch — MT.
    return isInteractive ? 1 : 8;
  }

  void ExecuteBatchMacro(const char *macroPath) {
    if (!uiManager) {
      uiManager = Geant4::G4UImanager::GetUIpointer();
    }

    if (!macroPath) {
      Geant4::G4Exception("Runner", "NullMacroPath",
                          Geant4::G4ExceptionSeverity::FatalException,
                          "Batch macro path is null.");

      return;
    }

    Geant4::G4String command = "/control/execute ";
    command += macroPath;

    uiManager->ApplyCommand(command);
  }

#pragma endregion

#pragma region Variables

private:
  std::unique_ptr<Geant4::G4UIExecutive> ui;
  std::unique_ptr<Geant4::G4MTRunManager> runManager;
  std::unique_ptr<Geant4::G4VisExecutive> visManager;

  Geant4::G4UImanager *uiManager = nullptr;

  std::unique_ptr<BetaSimLib::Detectors::DetectorManager> detManager;
  std::shared_ptr<BetaSimLib::Generators::GeneratorManager> genManager;

  std::unique_ptr<BetaSimLib::Messenger::BaseExperimentMessenger> expMessenger;

  std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig> config;
  std::shared_ptr<BetaSimLib::Runtime::ExperimentState> experimentState;

#pragma endregion
};

} // namespace BetaSimLib::Runner