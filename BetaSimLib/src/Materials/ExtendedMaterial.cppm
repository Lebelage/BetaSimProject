//
// Created by Sonora on 18.06.2026.
//
export module BetaSimLib.Materials.ExtendedMaterial;
import Geant4.Externals;
export namespace BetaSimLib::Materials {
    class ExtendedG4Material {
    public:
        ExtendedG4Material() {}

        ExtendedG4Material(Geant4::G4Material *material, float x, float Eg, bool isAlloy)
            : mat{material}, x{x}, Eg{Eg}, isAlloy{isAlloy} {
        };

        ~ExtendedG4Material() {
        };

        ExtendedG4Material& operator=(const ExtendedG4Material& other) = default;
    public:
        float GetX() { return x; }
        float GetEg() { return Eg; }
        Geant4::G4Material *GetG4Material() { return mat; }
        bool IsAlloy() { return isAlloy; }

    private:
        Geant4::G4Material *mat;
        float x;
        float Eg;
        bool isAlloy;
    };
};