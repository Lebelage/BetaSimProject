//
// Created by Sonora on 25.06.2026.
//
export module BetaSimLib.Detectors.StartDetector;
import std;
import Geant4.Externals;
export namespace BetaSimLib::Detectors {
    class StartDetector : public Geant4::G4VUserDetectorConstruction {

#pragma region Constructor

    public:
        StartDetector() {}

        ~StartDetector() override{};

        StartDetector(const StartDetector &) = delete;
        StartDetector &operator=(const StartDetector &) = delete;

        StartDetector(const StartDetector &&) = delete;
        StartDetector &operator=(const StartDetector &&) = delete;
#pragma endregion

#pragma region Methods
    public:
        Geant4::G4VPhysicalVolume *Construct() override {
            auto* nist = Geant4::G4NistManager::Instance();

            auto* worldMat = nist->FindOrBuildMaterial("G4_Galactic");

            auto* solidWorld = new Geant4::G4Box("World", 1, 1, 1);
            auto* logicWorld = new Geant4::G4LogicalVolume(
                solidWorld,
                worldMat,
                "WorldLV"
            );

            auto* physWorld = new Geant4::G4PVPlacement(
                nullptr,
                Geant4::G4ThreeVector(),
                logicWorld,
                "WorldPV",
                nullptr,
                false,
                0
            );

            return physWorld;

        };

        void Analyze() {};
#pragma endregion
    };
}
