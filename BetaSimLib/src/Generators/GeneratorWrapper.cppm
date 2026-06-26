//
// Created by Sonora on 26.06.2026.
//
export module BetaSimLib.Generators.GeneratorWrapper;

import std;
import Geant4.Externals;

import BetaSimLib.Models.Experiment;
import BetaSimLib.Runtime.ExperimentState;

export namespace BetaSimLib::Generators {

class GeneratorWrapper final : public Geant4::G4VUserPrimaryGeneratorAction {
#pragma region Constructors/Destructors

public:
    explicit GeneratorWrapper(std::shared_ptr<Runtime::ExperimentState> state)
        : state(std::move(state)),
          gun(std::make_unique<Geant4::G4ParticleGun>(1)) {
    }

    ~GeneratorWrapper() override = default;

    GeneratorWrapper(const GeneratorWrapper&) = delete;
    GeneratorWrapper& operator=(const GeneratorWrapper&) = delete;

    GeneratorWrapper(GeneratorWrapper&&) = delete;
    GeneratorWrapper& operator=(GeneratorWrapper&&) = delete;

#pragma endregion

#pragma region Methods

public:
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

        switch (cfg->sourceType) {
            case Models::SourceType::Gun:
                GenerateGun(event, *cfg);
                return;

            case Models::SourceType::Decay:
                Geant4::G4Exception(
                    "GeneratorWrapper",
                    "DecayNotImplemented",
                    Geant4::G4ExceptionSeverity::FatalException,
                    "Decay source is not implemented yet."
                );
                return;

            default:
                Geant4::G4Exception(
                    "GeneratorWrapper",
                    "UnknownSourceType",
                    Geant4::G4ExceptionSeverity::FatalException,
                    "Unknown source type."
                );
                return;
        }
    }

private:
    void GenerateGun(
        Geant4::G4Event* event,
        const Models::BaseExperimentConfig& cfg
    ) {
        auto* particle =
            Geant4::G4ParticleTable::GetParticleTable()
                ->FindParticle(cfg.gun.particle);

        if (!particle) {
            const auto message =
                std::string("Unknown particle: ") + cfg.gun.particle;

            Geant4::G4Exception(
                "GeneratorWrapper",
                "UnknownParticle",
                Geant4::G4ExceptionSeverity::FatalException,
                message.c_str()
            );

            return;
        }

        gun->SetParticleDefinition(particle);
        gun->SetParticleEnergy(cfg.gun.energy);
        gun->SetParticlePosition(cfg.gun.pos);
        gun->SetParticleMomentumDirection(cfg.gun.dir);

        gun->GeneratePrimaryVertex(event);
    }

#pragma endregion

#pragma region Variables

private:
    std::shared_ptr<Runtime::ExperimentState> state;
    std::unique_ptr<Geant4::G4ParticleGun> gun;

#pragma endregion
};

} // namespace BetaSimLib::Generators