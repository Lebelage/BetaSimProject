//
// Created by Sonora on 26.06.2026.
//
export module BetaSimLib.Actions.RunAction;
import Geant4.Externals;
export namespace BetaSimLib::Actions {
    class RunAction : public Geant4::G4UserRunAction {
#pragma region Constructors/Destructor

    public:
        RunAction() = default;

        ~RunAction() override = default;
#pragma endregion

#pragma region Methods
        void BeginOfRunAction(const Geant4::G4Run* run) override {}

        void EndOfRunAction(const Geant4::G4Run* run) override {}
#pragma endregion
    };
}
