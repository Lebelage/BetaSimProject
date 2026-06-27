//
// Created by Sonora on 26.06.2026.
//
export module BetaSimLib.Generators.BaseGenerator;

import std;
import Geant4.Externals;

import BetaSimLib.Runtime.ExperimentState;
import BetaSimLib.Models.Experiment;

export namespace BetaSimLib::Generators {

class BaseGenerator : public Geant4::G4VUserPrimaryGeneratorAction {
public:
    explicit BaseGenerator(
        std::shared_ptr<BetaSimLib::Runtime::ExperimentState> state
    )
        : state(std::move(state)) {
    }

    ~BaseGenerator() override = default;

    BaseGenerator(const BaseGenerator&) = delete;
    BaseGenerator& operator=(const BaseGenerator&) = delete;

    BaseGenerator(BaseGenerator&&) = delete;
    BaseGenerator& operator=(BaseGenerator&&) = delete;

protected:
    std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig> GetConfig() const {
        return state->GetConfig();
    }

private:
    std::shared_ptr<BetaSimLib::Runtime::ExperimentState> state;
};

} // namespace BetaSimLib::Generators