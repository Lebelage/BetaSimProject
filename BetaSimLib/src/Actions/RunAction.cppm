module;
#include "G4Threading.hh"
export module BetaSimLib.Actions.RunAction;

import Geant4.Externals;

import BetaSimLib.Statistics.SimulationStatisticsService;

export namespace BetaSimLib::Actions {

class RunAction final : public Geant4::G4UserRunAction {
public:
    explicit RunAction(bool manageVisualization = false)
        : manageVisualization(manageVisualization) {
    }

    ~RunAction() override = default;

public:
    void BeginOfRunAction(const Geant4::G4Run*) override {
        if (!G4Threading::IsMasterThread()) {
            return;
        }

        auto& stats =
            BetaSimLib::Statistics::SimulationStatisticsService::Instance();

        stats.Reset();

        stats.ConfigureElectronSpectrum(
            0.0 * Geant4::keV,
            100.0 * Geant4::keV,
            100
        );

        Geant4::cout()
            << "[RunAction] Statistics reset."
            << Geant4::endl;

        if (manageVisualization) {
            DisableVisualizationForRun();
        }
    }

    void EndOfRunAction(const Geant4::G4Run*) override {
        if (!G4Threading::IsMasterThread()) {
            return;
        }

        auto& stats =
            BetaSimLib::Statistics::SimulationStatisticsService::Instance();

        stats.PrintSummary();

        stats.ExportToCsvFiles("BetaSim_Output");

        if (manageVisualization) {
            RedrawVisualizationAfterRun();
        }
    }

private:
    void DisableVisualizationForRun() const {
        auto* uiManager = Geant4::G4UImanager::GetUIpointer();

        if (!uiManager) {
            return;
        }

        uiManager->ApplyCommand("/vis/disable");
        uiManager->ApplyCommand("/tracking/storeTrajectory 0");
    }

    void RedrawVisualizationAfterRun() const {
        auto* uiManager = Geant4::G4UImanager::GetUIpointer();

        if (!uiManager) {
            return;
        }

        uiManager->ApplyCommand("/vis/scene/clear");
        uiManager->ApplyCommand("/vis/scene/create");
        uiManager->ApplyCommand("/vis/scene/add/volume world");
        uiManager->ApplyCommand("/vis/sceneHandler/attach");
        uiManager->ApplyCommand("/vis/viewer/set/style surface");
        uiManager->ApplyCommand("/vis/enable");
        uiManager->ApplyCommand("/vis/viewer/refresh");
    }

private:
    bool manageVisualization = false;
};

}