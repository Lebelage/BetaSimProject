//
// Created by Sonora on 26.06.2026.
//
module;
#include "Randomize.hh"
export module BetaSimLib.Generators.RadioIsotopeGenerator;

import std;
import Geant4.Externals;

import BetaSimLib.Generators.BaseGenerator;
import BetaSimLib.Runtime.ExperimentState;
import BetaSimLib.Models.Experiment;

export namespace BetaSimLib::Generators {

class RadioIsotopeGenerator final : public BaseGenerator {
public:
    explicit RadioIsotopeGenerator(
        std::shared_ptr<BetaSimLib::Runtime::ExperimentState> state
    )
        : BaseGenerator(std::move(state)),
          gun(std::make_unique<Geant4::G4ParticleGun>(1)) {
    }

    ~RadioIsotopeGenerator() override = default;

    void GeneratePrimaries(Geant4::G4Event* event) override {
        auto cfg = GetConfig();

        if (!cfg) {
            Geant4::G4Exception(
                "RadioIsotopeGenerator",
                "NoExperimentConfig",
                Geant4::G4ExceptionSeverity::FatalException,
                "Experiment config is not set."
            );

            return;
        }

        auto* ion = Geant4::G4IonTable::GetIonTable()->GetIon(
            cfg->isotope.z,
            cfg->isotope.a,
            cfg->isotope.excitationEnergy
        );

        if (!ion) {
            Geant4::G4Exception(
                "RadioIsotopeGenerator",
                "IonNotFound",
                Geant4::G4ExceptionSeverity::FatalException,
                "Could not create ion for radioactive source."
            );

            return;
        }

        gun->SetParticleDefinition(ion);
        gun->SetParticleCharge(cfg->isotope.ionCharge);
        gun->SetParticleEnergy(0.0);

        gun->SetParticlePosition(
            GeneratePositionInsideSourceLayer(*cfg)
        );

        // Для иона в покое направление почти не важно,
        // но ParticleGun требует валидное направление.
        gun->SetParticleMomentumDirection(
            Geant4::G4ThreeVector(0.0, 0.0, 1.0)
        );

        gun->GeneratePrimaryVertex(event);
    }

private:
    Geant4::G4ThreeVector GeneratePositionInsideSourceLayer(
        const BetaSimLib::Models::BaseExperimentConfig& cfg
    ) const {
        const auto& source = cfg.sourceLayer;

        const double x = RandomUniform(
            -source.stackX / 2.0,
             source.stackX / 2.0
        );

        const double y = RandomUniform(
            -source.stackY / 2.0,
             source.stackY / 2.0
        );

        const double sourceThickness = GetTotalSourceThickness(source);

        const double z = RandomUniform(
            0.0,
            sourceThickness
        );

        return Geant4::G4ThreeVector(x, y, z);
    }

    double GetTotalSourceThickness(
        const BetaSimLib::Models::SourceLayerConfig& source
    ) const {
        double total = 0.0;

        for (const auto& layer : source.layers) {
            total += layer.thickness;
        }

        return total;
    }

    double RandomUniform(double min, double max) const {
        return min + (max - min) * G4UniformRand();
    }

private:
    std::unique_ptr<Geant4::G4ParticleGun> gun;
};

} // namespace BetaSimLib::Generators