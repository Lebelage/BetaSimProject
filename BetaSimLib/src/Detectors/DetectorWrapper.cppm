//
// Created by Sonora on 18.06.2026.
//
export module BetaSimLib.Detectors.DetectorWrapper;
import std;
import Geant4.Externals;
import BetaSimLib.Concepts.Detectors;
export namespace BetaSimLib::Detectors {
    class DetectorWrapper : public Geant4::G4VUserDetectorConstruction {
#pragma region Constructors/Destructors

    public:
        DetectorWrapper() = default;

        ~DetectorWrapper() override = default;
#pragma endregion

#pragma region Methods

        template<Concepts::Detectors::DetectorConstructionConcept Detector>
        void SetBuilder(std::unique_ptr<Detector> builder) {
            currentBuilder = std::move(builder);
        }

        Geant4::G4VPhysicalVolume *Construct() override {
            if (!currentBuilder) return nullptr;

            return currentBuilder->Construct();
        }

        void ConstructSDandField() override {
            if (currentBuilder) currentBuilder->ConstructSDandField();
        }
        
#pragma endregion

#pragma region Variables

    private:
        std::unique_ptr<G4VUserDetectorConstruction> currentBuilder;
#pragma endregion
    };
} // namespace BetaSimLib::Detectors
