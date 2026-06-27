module;

// 1. Базовые системные заголовочные файлы
#include <ostream>

// 2. Базовые типы, строки и системные утилиты Geant4
#include "G4Types.hh"
#include "G4String.hh"
#include "G4ios.hh"
#include "G4Exception.hh"
#include "G4ExceptionSeverity.hh"
#include "G4Tokenizer.hh"
#include "G4ThreeVector.hh"

// 3. Менеджеры ядра и интерфейс пользователя (UI)
#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4UImessenger.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"
#include "G4UIparameter.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"

// 4. Физические процессы и листы (Physics Lists)
#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"

// 5. Геометрия, материалы и объемы
#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Isotope.hh"
#include "G4Element.hh"

// 6. Логика симуляции (User Actions) и этапы
#include "G4VUserActionInitialization.hh"
#include "G4UserRunAction.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4Run.hh"
#include "G4Event.hh"
#include "G4Step.hh"
#include "G4StepStatus.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"

// 7. Чувствительные детекторы и хиты (SD & Hits)
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4VSensitiveDetector.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"

// 8. Сторонние библиотеки (CLHEP)
#include "CLHEP/Units/SystemOfUnits.h"

export module Geant4.Externals;

export namespace Geant4 {
    // ---------------------------------------------------------
    // Детекторы и хиты
    // ---------------------------------------------------------

    using ::G4HCofThisEvent;
    using ::G4SDManager;
    using ::G4VSensitiveDetector;
    using ::G4THitsCollection;
    using ::G4VHit;
    using ::G4Step;
    using ::G4TouchableHistory;
    using ::G4Track;

    // ---------------------------------------------------------
    // Step status
    // ---------------------------------------------------------

    using ::G4StepStatus;
    using ::fWorldBoundary;
    using ::fGeomBoundary;
    using ::fAtRestDoItProc;
    using ::fAlongStepDoItProc;
    using ::fPostStepDoItProc;
    using ::fUserDefinedLimit;
    using ::fExclusivelyForcedProc;
    using ::fUndefined;

    // ---------------------------------------------------------
    // Менеджеры, таблицы и геометрия
    // ---------------------------------------------------------

    using ::G4IonTable;
    using ::G4NistManager;
    using ::G4VPhysicalVolume;
    using ::G4VUserDetectorConstruction;
    using ::G4VUserActionInitialization;
    using ::G4MTRunManager;
    using ::G4RunManager;
    using ::G4UIExecutive;
    using ::G4UImanager;
    using ::G4VisExecutive;

    // ---------------------------------------------------------
    // Физика
    // ---------------------------------------------------------

    using ::FTFP_BERT;
    using ::G4EmStandardPhysics_option4;
    using ::G4StepLimiterPhysics;
    using ::G4DecayPhysics;
    using ::G4RadioactiveDecayPhysics;

    // ---------------------------------------------------------
    // Генераторы и действия пользователя
    // ---------------------------------------------------------

    using ::G4Event;
    using ::G4VUserPrimaryGeneratorAction;
    using ::G4ParticleGun;
    using ::G4ParticleTable;
    using ::G4Run;
    using ::G4UserRunAction;

    // ---------------------------------------------------------
    // UI команды и мессенджеры
    // ---------------------------------------------------------

    using ::G4UImessenger;
    using ::G4UIcmdWith3VectorAndUnit;
    using ::G4UIcmdWithAString;
    using ::G4UIcommand;
    using ::G4UIcmdWithADoubleAndUnit;
    using ::G4UIdirectory;
    using ::G4UIparameter;

    // ---------------------------------------------------------
    // Утилиты, строки и типы данных
    // ---------------------------------------------------------

    using ::G4Tokenizer;
    using ::G4LogicalVolume;
    using ::G4Box;
    using ::G4PVPlacement;
    using ::G4String;
    using ::G4Exception;
    using ::G4ExceptionSeverity;
    using ::G4Material;
    using ::G4Isotope;
    using ::G4Element;
    using ::G4double;
    using ::G4int;
    using ::G4bool;
    using ::G4ThreeVector;

    // ---------------------------------------------------------
    // Физические константы и единицы измерения CLHEP
    // ---------------------------------------------------------

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

    inline constexpr G4double mole = CLHEP::mole;
    inline constexpr G4double g    = CLHEP::g;

    // ---------------------------------------------------------
    // G4cout / G4endl wrappers
    // ---------------------------------------------------------

    inline std::ostream& cout() {
        return G4cout;
    }

    inline std::ostream& endl(std::ostream& os) {
        return os << G4endl;
    }
}