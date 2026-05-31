//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file DetectorConstruction.hh
/// \brief Definition of the B1::DetectorConstruction class

#ifndef B1DetectorConstruction_h
#define B1DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4UnionSolid.hh"
#include "G4SystemOfUnits.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4GenericMessenger;
class G4Material;
class G4UserLimits;

/// Detector construction class to define materials and geometry.

struct ConicalTargetGeometry
{
  G4VSolid* liquidSolid = nullptr;
  G4VSolid* nbOuterSolid = nullptr;

  G4double Rfront = 0.0;
  G4double Rback = 0.0;

  G4double coneLength = 0.0;
  G4double sphereRadius = 0.0;

  G4double totalLength = 0.0;

  G4double NbThickness = 0.0;
};


class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction();
    ~DetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;

    G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }

  protected:
    void ConstructTargetChamber();
    ConicalTargetGeometry BuildConicalLiquidTarget();
    G4UnionSolid* BuildSolidUnionTwo(G4double h, G4double r);
    G4Material* DefineLiquidTargetMaterial();
    G4Material* DefineHavarMaterial();
    G4Material* DefineHeliumGas();
    G4Material* CreateTargetMaterialFromCSV();
    void SetStepLimit(G4LogicalVolume*);
    G4bool fUsingTargetChamber{true};
    G4LogicalVolume* fScoringVolume = nullptr;
    G4LogicalVolume* fLogicWorld = nullptr;
    G4double targetSizeZ = 1.0;
    G4bool fUsingStepLimit = false;
    G4double fStepMax = 0.1;
    G4GenericMessenger* fMessenger;
    G4UserLimits* fStepLimit = nullptr;
    G4String fConcentration = "0.05M";
    G4double fTargetHNO3Molarity = 0;//same as fConcentration but double type
    G4String fCSVFileName = "";
    G4String fBeamEntranceWindowMatName = "havar";
    G4double fBeamEntranceWindowThickness = 35.0 *um;
    
};



//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
