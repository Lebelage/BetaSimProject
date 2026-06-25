module;
#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4RunManager.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4UImessenger.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"
#include "G4UIparameter.hh"
#include "G4String.hh"
#include "G4Exception.hh"
#include "G4ExceptionSeverity.hh"
#include "G4Tokenizer.hh"
#include "G4ThreeVector.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Isotope.hh"
#include "G4Element.hh"
#include "G4Types.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4MTRunManager.hh"
#include "G4VUserActionInitialization.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4NistManager.hh"




#include "CLHEP/Units/SystemOfUnits.h"
export module Geant4.Externals;
export namespace Geant4{
    using ::G4NistManager;
    using ::G4VPhysicalVolume;
    using ::G4VUserDetectorConstruction;
    using ::G4VUserActionInitialization;
    using ::G4MTRunManager;
    using ::G4RunManager;
    using ::G4UIExecutive;
    using ::G4UImanager;
    using ::G4VisExecutive;
    using ::FTFP_BERT;
    using ::G4EmStandardPhysics_option4;
    using ::G4StepLimiterPhysics;
    using ::G4DecayPhysics;
    using ::G4RadioactiveDecayPhysics;

    using ::G4UImessenger;
    using ::G4UIcmdWith3VectorAndUnit;
    using ::G4UIcmdWithAString;
    using ::G4UIcommand;
    using ::G4UIcmdWithADoubleAndUnit;
    using ::G4UIdirectory;
    using ::G4UIparameter;
    using ::G4Tokenizer;
    using ::G4LogicalVolume;
    using ::G4Box;
    using ::G4PVPlacement;

    using ::G4String;
    using ::G4Exception;
    using ::G4ExceptionSeverity;
    using ::G4NistManager;
    using ::G4Material;
    using ::G4Isotope;
    using ::G4Element;
    using ::G4double;
    using ::G4int;


    using ::G4ThreeVector;

    using ::G4VUserDetectorConstruction;

    inline constexpr G4double perCent = CLHEP::perCent;

    inline constexpr G4double um  = CLHEP::um;
    inline constexpr G4double mm  = CLHEP::mm;
    inline constexpr G4double cm  = CLHEP::cm;
    inline constexpr G4double m   = CLHEP::m;
    inline constexpr G4double cm2 = CLHEP::cm2;
    inline constexpr G4double cm3 = CLHEP::cm3;

    inline constexpr G4double eV  = CLHEP::eV;
    inline constexpr G4double keV = CLHEP::keV;
    inline constexpr G4double MeV = CLHEP::MeV;

    inline constexpr G4double mole  = CLHEP::mole;
    inline constexpr G4double g = CLHEP::g;
}