export module BetaSimLib.Concepts.Detectors;
import std;
import Geant4.Externals;
export namespace BetaSimLib::Concepts::Detectors {

    template<typename T>
    concept DetectorConstructionConcept =
            std::derived_from<T, Geant4::G4VUserDetectorConstruction> && requires(T det)
            {
                { det.Construct() } -> std::same_as<Geant4::G4VPhysicalVolume *>;
                { det.ConstructSDandField() } -> std::same_as<void>;
                { det.Analyze() } -> std::same_as<void>;
            };
}