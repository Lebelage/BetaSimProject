//
// Created by Sonora on 26.06.2026.
//
export module BetaSimLib.Generators.GeneratorManager;

import std;
import Geant4.Externals;

import BetaSimLib.Runtime.ExperimentState;
import BetaSimLib.Generators.GeneratorWrapper;

export namespace BetaSimLib::Generators {

    class GeneratorManager {
#pragma region Constructors/Destructors

    public:
        explicit GeneratorManager(std::shared_ptr<Runtime::ExperimentState> state)
            : state(std::move(state)) {
        }

        ~GeneratorManager() = default;

        GeneratorManager(const GeneratorManager&) = delete;
        GeneratorManager& operator=(const GeneratorManager&) = delete;

        GeneratorManager(GeneratorManager&&) = delete;
        GeneratorManager& operator=(GeneratorManager&&) = delete;

#pragma endregion

#pragma region Methods

    public:
        Geant4::G4VUserPrimaryGeneratorAction* CreateGeneratorForWorker() const {
            return new GeneratorWrapper(state);
        }

#pragma endregion

#pragma region Variables

    private:
        std::shared_ptr<Runtime::ExperimentState> state;

#pragma endregion
    };

} // namespace BetaSimLib::Generators