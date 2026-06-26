//
// Created by Sonora on 26.06.2026.
//

export module BetaSimLib.Concepts.Generators.GeneratorConcept;
import std;
import Geant4.Externals;
export namespace BetaSimLib::Concepts::Generators {
    template <typename T>
    concept SourceGeneratorConcept =
        std::derived_from<T, Geant4::G4VUserPrimaryGeneratorAction> && requires(T source, Geant4::G4Event* event) {
        { source.GeneratePrimaries(event) } -> std::same_as<void>;
        };
}
