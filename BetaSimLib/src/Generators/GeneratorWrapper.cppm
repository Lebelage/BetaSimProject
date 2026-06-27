//
// Created by Sonora on 26.06.2026.
//
export module BetaSimLib.Generators.GeneratorWrapper;

import std;
import Geant4.Externals;

import BetaSimLib.Models.Experiment;
import BetaSimLib.Runtime.ExperimentState;
import BetaSimLib.Concepts.Generators.GeneratorConcept;

import BetaSimLib.Generators.BaseGenerator;
import BetaSimLib.Generators.GunGenerator;
import BetaSimLib.Generators.RadioIsotopeGenerator;

export namespace BetaSimLib::Generators {

class GeneratorWrapper final : public Geant4::G4VUserPrimaryGeneratorAction {
public:
    explicit GeneratorWrapper(
        std::shared_ptr<BetaSimLib::Runtime::ExperimentState> state
    )
        : state(std::move(state)) {
    }

    ~GeneratorWrapper() override = default;

    GeneratorWrapper(const GeneratorWrapper&) = delete;
    GeneratorWrapper& operator=(const GeneratorWrapper&) = delete;

    GeneratorWrapper(GeneratorWrapper&&) = delete;
    GeneratorWrapper& operator=(GeneratorWrapper&&) = delete;

public:
    template<BetaSimLib::Concepts::Generators::SourceGeneratorConcept Generator>
    void SetSourceGenerator(std::unique_ptr<Generator> generator) {
        currentGenerator = std::move(generator);
    }

    void GeneratePrimaries(Geant4::G4Event* event) override {
        auto cfg = state->GetConfig();

        if (!cfg) {
            Geant4::G4Exception(
                "GeneratorWrapper",
                "NoExperimentConfig",
                Geant4::G4ExceptionSeverity::FatalException,
                "Experiment config is not set."
            );

            return;
        }

        EnsureGeneratorIsActual(cfg);

        if (!currentGenerator) {
            Geant4::G4Exception(
                "GeneratorWrapper",
                "NoSourceGenerator",
                Geant4::G4ExceptionSeverity::FatalException,
                "Source generator is not set."
            );

            return;
        }

        currentGenerator->GeneratePrimaries(event);
    }

private:
    void EnsureGeneratorIsActual(
        const std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig>& cfg
    ) {
        const bool configChanged = observedConfig != cfg;
        const bool typeChanged =
            !activeSourceType.has_value() ||
            activeSourceType.value() != cfg->sourceType;

        if (!currentGenerator || configChanged || typeChanged) {
            RebuildGenerator(cfg);
        }
    }

    void RebuildGenerator(
        const std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig>& cfg
    ) {
        observedConfig = cfg;
        activeSourceType = cfg->sourceType;

        switch (cfg->sourceType) {
            case BetaSimLib::Models::SourceType::Gun:
                SetSourceGenerator(
                    std::make_unique<GunGenerator>(state)
                );
                break;

            case BetaSimLib::Models::SourceType::Decay:
                SetSourceGenerator(
                    std::make_unique<RadioIsotopeGenerator>(state)
                );
                break;

            default:
                Geant4::G4Exception(
                    "GeneratorWrapper",
                    "UnknownSourceType",
                    Geant4::G4ExceptionSeverity::FatalException,
                    "Unknown source type."
                );
                break;
        }
    }

private:
    std::shared_ptr<BetaSimLib::Runtime::ExperimentState> state;

    std::unique_ptr<Geant4::G4VUserPrimaryGeneratorAction> currentGenerator;

    std::shared_ptr<const BetaSimLib::Models::BaseExperimentConfig> observedConfig;
    std::optional<BetaSimLib::Models::SourceType> activeSourceType;
};

} // namespace BetaSimLib::Generators