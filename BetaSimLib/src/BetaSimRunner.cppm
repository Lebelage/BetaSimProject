export module BetaSimLib.Runner;
import std;
import Geant4.Externals;
import BetaSimLib.Commands.CommandBuilder;
import BetaSimLib.Detectors.DetectorManager;
import BetaSimLib.Detectors.StartDetector;
import BetaSimLib.EventDispatcher;
import BetaSimLib.Models.Experiment;
import BetaSimLib.Messenger;
import BetaSimLib.Actions.RunAction;
import BetaSimLib.Generators.GeneratorManager;
import BetaSimLib.Runtime.ExperimentState;
export namespace BetaSimLib::Runner {
    class UserActionInitialization final : public Geant4::G4VUserActionInitialization {
#pragma region Constructors/Destructors

    public:
        UserActionInitialization(
            std::shared_ptr<Runtime::ExperimentState> state,
            std::shared_ptr<Generators::GeneratorManager> generatorManager
        )
            : state(std::move(state)),
              generatorManager(std::move(generatorManager)) {
        }
        ~UserActionInitialization() override = default;

        UserActionInitialization(const UserActionInitialization &) = delete;
        UserActionInitialization &operator=(const UserActionInitialization &) = delete;

        UserActionInitialization(UserActionInitialization &&) = delete;
        UserActionInitialization &operator=(UserActionInitialization &&) = delete;

#pragma endregion

#pragma region Methods

    public:
        void BuildForMaster() const override {
            SetUserAction(new Actions::RunAction());
        }

        void Build() const override {
            SetUserAction(new Actions::RunAction());

            // Каждый worker thread получает свой PrimaryGeneratorAction.
            SetUserAction(generatorManager->CreateGeneratorForWorker());

            // Потом сюда добавишь:
            // SetUserAction(new Actions::EventAction(state));
            // SetUserAction(new Actions::SteppingAction(state));
        }

#pragma endregion

#pragma region Variables

    private:
        std::shared_ptr<Runtime::ExperimentState> state;
        std::shared_ptr<Generators::GeneratorManager> generatorManager;

#pragma endregion
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
        Runner() {
        }

        ~Runner() {
        }
#pragma endregion

#pragma region Methods

    public:
        void Initialize(int argc, char **argv) {
            // this->config = std::make_shared<BaseExperimentConfig>();
            InitializeRunManager(argc, argv);

            BetaSimLib::EventDispatcher::EventDispatcher::GetExperimentConfigReadyEvent().Add(
                [this](std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig> cfg) {
                    OnExperimentUpdated(cfg);
                });
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
            const bool isInteractive = argc == 1;

            if (isInteractive) {
                ui = std::make_unique<Geant4::G4UIExecutive>(argc, argv);
            }

            runManager = std::make_unique<Geant4::G4MTRunManager>();

            expMessenger =
                    std::make_unique<BetaSimLib::Messenger::BaseExperimentMessenger>();

            experimentState =
                    std::make_shared<BetaSimLib::Runtime::ExperimentState>();

            config =
                    std::make_shared<const BetaSimLib::Models::BaseExperimentConfig>();

            experimentState->SetConfig(config);

            detManager =
                    std::make_unique<BetaSimLib::Detectors::DetectorManager>();

            detManager->SetDetector(
                std::make_unique<BetaSimLib::Detectors::StartDetector>()
            );

            genManager = std::make_shared<BetaSimLib::Generators::GeneratorManager>(
                experimentState
            );

            runManager->SetNumberOfThreads(8);

            runManager->SetUserInitialization(
                detManager->ReleaseDetectorToGeant4()
            );

            runManager->SetUserInitialization(
                InitializePhysics()
            );

            runManager->SetUserInitialization(
                new UserActionInitialization(experimentState, genManager)
            );

            runManager->Initialize();
        }

        Geant4::FTFP_BERT *InitializePhysics() {
            auto *physics = new Geant4::FTFP_BERT();
            physics->ReplacePhysics(new Geant4::G4EmStandardPhysics_option4());
            physics->RegisterPhysics(new Geant4::G4StepLimiterPhysics());
            physics->RegisterPhysics(new Geant4::G4DecayPhysics());
            physics->RegisterPhysics(new Geant4::G4RadioactiveDecayPhysics());

            return physics;
        }
#pragma endregion

#pragma region Handlers
        void OnExperimentUpdated(
        std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig> newConfig) {
            if (!newConfig) {
                Geant4::G4Exception(
                    "Runner",
                    "NullExperimentConfig",
                    Geant4::G4ExceptionSeverity::JustWarning,
                    "Received null experiment config."
                );

                return;
            }

            config = std::move(newConfig);

            experimentState->SetConfig(config);

            detManager->ApplyConfig(config);

            if (runManager) {
                runManager->GeometryHasBeenModified();
            }

            if (uiManager) {
                uiManager->ApplyCommand("/vis/scene/notifyHandlers");
            }
        }
#pragma endregion

#pragma region Variables

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
