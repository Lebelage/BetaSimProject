//
// Created by Sonora on 26.06.2026.
//
export module BetaSimLib.Generators.GeneratorManager;

import std;
import Geant4.Externals;

import BetaSimLib.Models.Experiment;
import BetaSimLib.Runtime.ExperimentState;
import BetaSimLib.Generators.GeneratorWrapper;

export namespace BetaSimLib::Generators {

class GeneratorManager {
public:
    explicit GeneratorManager(
        std::shared_ptr<BetaSimLib::Runtime::ExperimentState> state
    )
        : state(std::move(state)) {
    }

    ~GeneratorManager() = default;

    GeneratorManager(const GeneratorManager&) = delete;
    GeneratorManager& operator=(const GeneratorManager&) = delete;

    GeneratorManager(GeneratorManager&&) = delete;
    GeneratorManager& operator=(GeneratorManager&&) = delete;

public:
    void ApplyConfig(
        std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig> config
    ) {
        currentConfig = std::move(config);

        // Важно:
        // уже созданные worker GeneratorWrapper не заменяются здесь.
        // Они сами увидят новый config через ExperimentState
        // и пересоздадут свой внутренний generator в GeneratePrimaries().
    }

    Geant4::G4VUserPrimaryGeneratorAction* CreateGeneratorForWorker() const {
        return new GeneratorWrapper(state);
    }

private:
    std::shared_ptr<BetaSimLib::Runtime::ExperimentState> state;
    std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig> currentConfig;
};

} // namespace BetaSimLib::Generators