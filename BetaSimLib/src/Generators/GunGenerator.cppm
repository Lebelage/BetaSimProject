//
// Created by Sonora on 26.06.2026.
//
export module BetaSimLib.Generators.GunGenerator;

import std;
import Geant4.Externals;

import BetaSimLib.Generators.BaseGenerator;
import BetaSimLib.Runtime.ExperimentState;
import BetaSimLib.Models.Experiment;

export namespace BetaSimLib::Generators {

class GunGenerator final : public BaseGenerator {
public:
    explicit GunGenerator(
        std::shared_ptr<BetaSimLib::Runtime::ExperimentState> state
    )
        : BaseGenerator(std::move(state)),
          gun(std::make_unique<Geant4::G4ParticleGun>(1)) {
    }

    ~GunGenerator() override = default;

    void GeneratePrimaries(Geant4::G4Event* event) override {
        auto cfg = GetConfig();

        if (!cfg) {
            Geant4::G4Exception(
                "GunGenerator",
                "NoExperimentConfig",
                Geant4::G4ExceptionSeverity::FatalException,
                "Experiment config is not set."
            );

            return;
        }

        auto* particle =
            Geant4::G4ParticleTable::GetParticleTable()
                ->FindParticle(cfg->gun.particle);

        if (!particle) {
            const auto message =
                std::string("Unknown particle: ") + cfg->gun.particle;

            Geant4::G4Exception(
                "GunGenerator",
                "UnknownParticle",
                Geant4::G4ExceptionSeverity::FatalException,
                message.c_str()
            );

            return;
        }

        gun->SetParticleDefinition(particle);
        gun->SetParticleEnergy(cfg->gun.energy);
        gun->SetParticlePosition(cfg->gun.pos);
        gun->SetParticleMomentumDirection(cfg->gun.dir);

        gun->GeneratePrimaryVertex(event);
    }

private:
    std::unique_ptr<Geant4::G4ParticleGun> gun;
};

} // namespace BetaSimLib::Generators