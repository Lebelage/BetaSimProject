#include <memory>
import std;
import Geant4.Externals;
import BetaSimLib.Commands.CommandBuilder;
import BetaSimLib.Detectors.DetectorManager;
import BetaSimLib.Detectors.StartDetector;
namespace BetaSimLib::Runner {

class UserActionInitialization : Geant4::G4VUserActionInitialization {
public:
  UserActionInitialization() = default;
  ~UserActionInitialization() = default;

  public:
        virtual void BuildForMaster() const override
        {

        }

        virtual void Build() const override
        {
                
        }
};

class Runner {
public:
#pragma region Singleton
  static Runner &GetInstance() {
    static Runner instance;
    return instance;
  }

  Runner(const Runner &) = delete;

  Runner &operator=(const Runner &) = delete;

  Runner(const Runner &&) = delete;

  Runner &operator=(const Runner &&) = delete;
#pragma endregion

#pragma region Constructors/Destructor
private:
  Runner() {}
  ~Runner() {}
#pragma endregion

#pragma region Methods
public:
  void Initialize(int argc, char **argv) {

    // this->config = std::make_shared<BaseExperimentConfig>();
    InitializeRunManager(argc, argv);
  }

  void InitializeUI(int argc, char **argv) {
    uiManager = Geant4::G4UImanager::GetUIpointer();

    const auto isInteractive = (argc == 1);
    if (!isInteractive) {

      std::string fullCommand;
      Geant4::G4String command = "/control/execute ";
      fullCommand += command;
      fullCommand += argv[1];
      uiManager->ApplyCommand(Geant4::G4String(fullCommand));
      return;
    }

    visManager = std::make_unique<Geant4::G4VisExecutive>();
    visManager->Initialize();

    uiManager->ApplyCommand("/control/macroPath AppConfigs");
    uiManager->ApplyCommand("/control/execute init.mac");

    ui->SessionStart();
  }

private:
  void InitializeRunManager(int argc, char **argv) {
    const bool isInteractive = (argc == 1);

    if (isInteractive)
      ui = std::make_unique<Geant4::G4UIExecutive>(argc, argv);

    runManager = std::make_unique<Geant4::G4MTRunManager>();
    //expMessenger = std::make_unique<BaseExperimentMessenger>();

    detManager = std::make_unique<BetaSimLib::Detectors::DetectorManager>();
    detManager->SetDetector(std::make_unique<BetaSimLib::Detectors::StartDetector>());

    runManager->SetNumberOfThreads(8);

    runManager->SetUserInitialization(detManager->GetCurrentDetectorPointer());

    //     detManager->SetDetector(std::make_unique<StartDetector>());

    //     auto *physics = InitializePhysics();
    //     runManager->SetUserInitialization(physics);

    //     sourceManager = std::make_unique<SourceGeneratorManager>();
    //     sourceManager->SetSourceGenerator(std::make_unique<BaseSourceGenerator>(nullptr));

    //     runManager->SetUserInitialization(detManager->GetCurrentDetectorPointer());
    //     runManager->SetUserAction(sourceManager->GetCurrentSourceGeneratorPointer());

    //     runManager->SetUserAction(new
    //     GeantCore::Core::Actions::BaseRunAction());
    //     //runManager->SetUserAction(new
    //     GeantCore::Core::Actions::BaseSteppingAction());

    //     runManager->Initialize();
  };

#pragma endregion

#pragma region Variables

  std::unique_ptr<Geant4::G4UIExecutive> ui;
  std::unique_ptr<Geant4::G4MTRunManager> runManager;
  std::unique_ptr<Geant4::G4VisExecutive> visManager;
  Geant4::G4UImanager *uiManager = nullptr;

  std::unique_ptr<BetaSimLib::Detectors::DetectorManager> detManager;

#pragma endregion
};
} // namespace BetaSimLib::Runner