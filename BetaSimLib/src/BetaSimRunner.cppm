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

class UserActionInitialization final
    : public Geant4::G4VUserActionInitialization {
public:
    UserActionInitialization(
        std::shared_ptr<BetaSimLib::Runtime::ExperimentState> state,
        std::shared_ptr<BetaSimLib::Generators::GeneratorManager> generatorManager,
        bool manageVisualization
    )
        : state(std::move(state)),
          generatorManager(std::move(generatorManager)),
          manageVisualization(manageVisualization) {
    }

    ~UserActionInitialization() override = default;

    UserActionInitialization(const UserActionInitialization&) = delete;
    UserActionInitialization& operator=(const UserActionInitialization&) = delete;

    UserActionInitialization(UserActionInitialization&&) = delete;
    UserActionInitialization& operator=(UserActionInitialization&&) = delete;

public:
    void BuildForMaster() const override {
        SetUserAction(
            new BetaSimLib::Actions::RunAction(manageVisualization)
        );
    }

    void Build() const override {
        // Worker threads не должны трогать /vis.
        SetUserAction(
            new BetaSimLib::Actions::RunAction(false)
        );

        SetUserAction(
            generatorManager->CreateGeneratorForWorker()
        );

        // Потом сюда:
        // SetUserAction(new BetaSimLib::Actions::EventAction(state));
        // SetUserAction(new BetaSimLib::Actions::SteppingAction(state));
    }

private:
    std::shared_ptr<BetaSimLib::Runtime::ExperimentState> state;
    std::shared_ptr<BetaSimLib::Generators::GeneratorManager> generatorManager;

    bool manageVisualization = false;
};

class Runner {
#pragma region Singleton

public:
    static Runner& GetInstance() {
        static Runner instance;
        return instance;
    }

    Runner(const Runner&) = delete;
    Runner& operator=(const Runner&) = delete;

    Runner(Runner&&) = delete;
    Runner& operator=(Runner&&) = delete;

#pragma endregion

#pragma region Constructors/Destructor

private:
    Runner() = default;
    ~Runner() = default;

#pragma endregion

#pragma region Public Methods

public:
    void Initialize(int argc, char** argv) {
        InitializeRunManager(argc, argv);
        SubscribeEvents();
    }

    void InitializeUI(int argc, char** argv) {
        uiManager = Geant4::G4UImanager::GetUIpointer();

        const bool isInteractive = IsInteractive(argc, argv);

        if (!isInteractive) {
            ExecuteBatchMacro(argv[1]);
            return;
        }
        

        visManager = std::make_unique<Geant4::G4VisExecutive>();
        visManager->Initialize();

        uiManager->ApplyCommand("/control/macroPath AppConfigs");

        // init.mac должен только открыть viewer.
        // Не вызывай здесь /vis/drawVolume, /vis/scene/add/volume,
        // /tracking/storeTrajectory 1 или /run/beamOn.
        uiManager->ApplyCommand("/control/execute init.mac");

        ui->SessionStart();
    }

#pragma endregion

#pragma region Initialization

private:
    void InitializeRunManager(int argc, char** argv) {
        const bool isInteractive = IsInteractive(argc, argv);

        if (isInteractive) {
            // Для режима "./Application ui 8" не передаём ui/8 в G4UIExecutive.
            int uiArgc = 1;
            ui = std::make_unique<Geant4::G4UIExecutive>(uiArgc, argv);
        }

        
        runManager = std::make_unique<Geant4::G4MTRunManager>();
        runManager->SetNumberOfThreads(8);

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

        genManager =
            std::make_shared<BetaSimLib::Generators::GeneratorManager>(
                experimentState
            );

        const auto threadCount = ResolveThreadCount(argc, argv);

        runManager->SetNumberOfThreads(threadCount);

        Geant4::cout()
            << "[Runner] Thread count = "
            << threadCount
            << Geant4::endl;

        runManager->SetUserInitialization(
            detManager->GetCurrentDetectorPointer()
        );

        runManager->SetUserInitialization(
            InitializePhysics()
        );

        // ВАЖНО:
        // Для UI-режима включаем защиту visualization всегда,
        // даже при 1 потоке, потому что World mismatch может случиться
        // и без MT, если scene создана до reinitialize geometry.
        const bool manageVisualization = isInteractive;

        runManager->SetUserInitialization(
            new UserActionInitialization(
                experimentState,
                genManager,
                manageVisualization
            )
        );

        runManager->Initialize();
    }

    Geant4::FTFP_BERT* InitializePhysics() {
        auto* physics = new Geant4::FTFP_BERT();

        physics->ReplacePhysics(
            new Geant4::G4EmStandardPhysics_option4()
        );

        physics->RegisterPhysics(
            new Geant4::G4StepLimiterPhysics()
        );

        // FTFP_BERT уже содержит обычный DecayPhysics.
        // Не добавляй G4DecayPhysics второй раз.
        //
        // physics->RegisterPhysics(new Geant4::G4DecayPhysics());

        physics->RegisterPhysics(
            new Geant4::G4RadioactiveDecayPhysics()
        );

        return physics;
    }

    void SubscribeEvents() {
        BetaSimLib::EventDispatcher::EventDispatcher
            ::GetExperimentConfigReadyEvent()
            .Add(
                [this](
                    std::shared_ptr<
                        const BetaSimLib::Models::BaseExperimentConfig
                    > cfg
                ) {
                    OnExperimentUpdated(std::move(cfg));
                }
            );
    }

#pragma endregion

#pragma region Event Handlers

private:
    void OnExperimentUpdated(
        std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig> newConfig
    ) {
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

        // ВАЖНО:
        // Перед заменой geometry выключаем visualization и чистим scene.
        // Иначе viewer может держать старый world и потом упасть с:
        // "World mis-match - not possible(!?)"
        DisableVisualizationBeforeGeometryUpdate();

        detManager->ApplyConfig(config);
        genManager->ApplyConfig(config);

        if (runManager) {
            runManager->GeometryHasBeenModified();
            runManager->ReinitializeGeometry();
        }

        Geant4::cout()
            << "[Runner] Geometry update scheduled. "
            << "Viewer will be redrawn after the next run."
            << Geant4::endl;

        // НЕ вызываем здесь RedrawGeometryIfInteractive().
        // Перерисовка должна быть после /run/beamOn,
        // когда Geant4 уже реально пересобрал world.
    }

#pragma endregion

#pragma region Visualization Helpers

private:
    void DisableVisualizationBeforeGeometryUpdate() {
        if (!uiManager) {
            uiManager = Geant4::G4UImanager::GetUIpointer();
        }

        if (!uiManager) {
            return;
        }

        uiManager->ApplyCommand("/vis/disable");
        uiManager->ApplyCommand("/vis/scene/clear");
        uiManager->ApplyCommand("/tracking/storeTrajectory 0");
    }

#pragma endregion

#pragma region Helpers

private:
    static bool IsInteractive(int argc, char** argv) {
        if (argc == 1) {
            return true;
        }

        if (argc >= 2 && std::string(argv[1]) == "ui") {
            return true;
        }

        return false;
    }

    int ResolveThreadCount(int argc, char** argv) const {
        // Режимы:
        //
        // ./Application
        //      Qt UI, 1 thread
        //
        // ./Application ui 8
        //      Qt UI, 8 threads
        //
        // ./Application batch.mac
        //      batch, 8 threads
        //
        // ./Application batch.mac 4
        //      batch, 4 threads

        if (argc >= 3) {
            try {
                const int requestedThreads = std::stoi(argv[2]);
                return std::max(1, requestedThreads);
            } catch (...) {
                Geant4::G4Exception(
                    "Runner",
                    "InvalidThreadArgument",
                    Geant4::G4ExceptionSeverity::JustWarning,
                    "Invalid thread count argument. Falling back to default thread count."
                );
            }
        }

        const bool isInteractive = IsInteractive(argc, argv);

        return isInteractive ? 1 : 8;
    }

    void ExecuteBatchMacro(const char* macroPath) {
        if (!uiManager) {
            uiManager = Geant4::G4UImanager::GetUIpointer();
        }

        if (!macroPath) {
            Geant4::G4Exception(
                "Runner",
                "NullMacroPath",
                Geant4::G4ExceptionSeverity::FatalException,
                "Batch macro path is null."
            );

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

    Geant4::G4UImanager* uiManager = nullptr;

    std::unique_ptr<BetaSimLib::Detectors::DetectorManager> detManager;
    std::shared_ptr<BetaSimLib::Generators::GeneratorManager> genManager;

    std::unique_ptr<BetaSimLib::Messenger::BaseExperimentMessenger> expMessenger;

    std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig> config;
    std::shared_ptr<BetaSimLib::Runtime::ExperimentState> experimentState;

#pragma endregion
};

} // namespace BetaSimLib::Runner