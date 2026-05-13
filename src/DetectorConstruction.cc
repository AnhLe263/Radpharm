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
/// \file DetectorConstruction.cc
/// \brief Implementation of the B1::DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Tubs.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Trd.hh"
#include "G4Element.hh"
#include "G4GenericMessenger.hh"
#include "G4UnitsTable.hh"
#include "G4UserLimits.hh"


DetectorConstruction::DetectorConstruction():G4VUserDetectorConstruction()
{
  fMessenger = new G4GenericMessenger(this,"/target/","");
  fMessenger->DeclarePropertyWithUnit("thickness","mm",targetSizeZ,"");
  fMessenger->DeclareProperty("usingStepMax",fUsingStepLimit,"");
  fMessenger->DeclarePropertyWithUnit("stepMax","mm",fStepMax,"");
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();
  // some needed material
  //Ni-64:
  G4double a = 63.927966*g/mole;
  G4Isotope* isoNi64 = new G4Isotope("Ni64", 28, 64, a);
  G4Element* elNi64  = new G4Element("Ni64","Ni64" , 1);
  elNi64->AddIsotope(isoNi64,100.*perCent);
  G4double density = 8.907*g/cm3;
  G4Material* Ni64 = new G4Material("Ni64", density, 1);
  Ni64->AddElement(elNi64, 1);

   //Ni-Zn68:
  a = 67.9248*g/mole;
  G4Isotope* isoZn68 = new G4Isotope("Zn68", 30, 68, a);
  G4Element* elZn68  = new G4Element("Zn68","Zn68" , 1);
  elZn68->AddIsotope(isoZn68,100.*perCent);
  density = 7.14*g/cm3;
  G4Material* Zn68 = new G4Material("Zn68", density, 1);
  Zn68->AddElement(elZn68, 1);
  
  
  // Envelope parameters
  //
  G4double env_sizeXY = 20 * cm, env_sizeZ = 30 * cm;

  // Option to switch on/off checking of volumes overlaps
  //
  G4bool checkOverlaps = true;

  //
  // World
  //
  G4double world_sizeXY = 1.2 * env_sizeXY;
  G4double world_sizeZ = 1.2 * env_sizeZ;
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_Galactic");

  auto solidWorld =
    new G4Box("World",  // its name
              0.5 * world_sizeXY, 0.5 * world_sizeXY, 0.5 * world_sizeZ);  // its size

  fLogicWorld = new G4LogicalVolume(solidWorld,  // its solid
                                        world_mat,  // its material
                                        "World");  // its name

  auto physWorld = new G4PVPlacement(nullptr,  // no rotation
                                     G4ThreeVector(),  // at (0,0,0)
                                     fLogicWorld,  // its logical volume
                                     "World",  // its name
                                     nullptr,  // its mother  volume
                                     false,  // no boolean operation
                                     0,  // copy number
                                     checkOverlaps);  // overlaps checking

  

  // Target
  G4LogicalVolume *logicTarget = nullptr;
  if (fUsingTargetChamber) {
    ConstructTargetChamber();
  } else {
    G4double targetSizeXY = 1.0*cm;
    G4cout<<" == > Target thickness = "<<targetSizeZ/mm<<"mm"<<G4endl;
    auto solidTarget =
      new G4Box("Target",targetSizeXY/2.,targetSizeXY/2.,targetSizeZ/2.);  

    logicTarget = new G4LogicalVolume(solidTarget,  // its solid
                                          Zn68,  // its material
                                          "Target");  // its name

    new G4PVPlacement(nullptr,  // no rotation
                      G4ThreeVector(),  // at position
                      logicTarget,  // its logical volume
                      "Target",  // its name
                      fLogicWorld,  // its mother  volume
                      false,  // no boolean operation
                      0,  // copy number
                      checkOverlaps);  // overlaps checking
    fScoringVolume = logicTarget;
  }
  // 
  //
  

   // print the table of materials
  G4cout << *(G4Material::GetMaterialTable()) << G4endl;
  //
  // always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructTargetChamber()
{
  G4double fTargetChamber_h = 3.14 *mm;
  G4double fTargetChamber_r = 8.5 *mm;
  G4double Nb_thickness = 5*mm;
  G4double Nb_h = fTargetChamber_h + Nb_thickness;
  G4double Nb_r = fTargetChamber_r + Nb_thickness;
  G4double TiRadius = Nb_r;
  G4double TisizeZ = 75. *um;
  G4NistManager* nist = G4NistManager::Instance();
  auto Ti = nist->FindOrBuildMaterial("G4_Ti");
  
  // --- 1. Titanium Window (Vacuum side) ---
  auto Tisolid =
      new G4Tubs("Ta",0,TiRadius,TisizeZ/2.,0,360*deg);  

  auto logicTi = new G4LogicalVolume(Tisolid,  // its solid
                                          Ti,  // its material
                                          "Tilayer");  // its name
  SetStepLimit(logicTi);

  G4ThreeVector Tipos(0,0,0*cm);
  new G4PVPlacement(nullptr,  // no rotation
                    Tipos,  // at position
                    logicTi,  // its logical volume
                    "Tilayer",  // its name
                    fLogicWorld,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    true);  // overlaps checking
  
  G4cout<<"====> On Z-axis, Zmin of Ti: "<<G4BestUnit(Tipos.getZ() - TisizeZ/2.,"Length")<<G4endl;
  // --- 2. Helium Gap (Cooling layer) ---
  auto He = DefineHeliumGas();
  auto heLayerRadius = Nb_r;
  G4double heGapThickness = 2.0 * mm;
  G4Tubs* solidHe = new G4Tubs("SolidHe", 0, heLayerRadius, 0.5*heGapThickness, 0, 360*deg);
  G4LogicalVolume* logicHe = new G4LogicalVolume(solidHe, He, "Helayer"); 
  SetStepLimit(logicHe);
  // Position of Helium Gap: Shifted by (Ti_thick/2 + He_thick/2)
  G4double zHe = Tipos.getZ() + 0.5*TisizeZ + 0.5*heGapThickness;
  G4cout<<"====> On Z-axis, Zmin of He: "<<G4BestUnit(zHe - 0.5*heGapThickness,"Length")<<G4endl;
  new G4PVPlacement(0, G4ThreeVector(0, 0, zHe), 
                  logicHe, "Helayer", fLogicWorld, false, 0, true);
  
  //Havar is right against the solution
  G4Material* Havar = DefineHavarMaterial();
  G4double HavarsizeZ = 38. *um; // 38 um tương ứng 0.0015" từ nhà sản xuất Hamilton Precision Metals
  auto Havarsolid =
      new G4Tubs("Havar",0,Nb_r,HavarsizeZ/2.,0,360*deg);  

  auto logicHavar = new G4LogicalVolume(Havarsolid,  // its solid
                                          Havar,  // its material
                                          "Havarlayer");  // its name
  SetStepLimit(logicHavar);
  G4double zHavar = zHe + 0.5*heGapThickness + 0.5*HavarsizeZ;
  G4cout<<"====> On Z-axis, Zmin of Havar: "<<G4BestUnit(zHavar - 0.5*HavarsizeZ,"Length")<<G4endl;
  G4ThreeVector Havarpos(0,0,zHavar);
  new G4PVPlacement(nullptr,  // no rotation
                    Havarpos,  // at position
                    logicHavar,  // its logical volume
                    "Havarlayer",  // its name
                    fLogicWorld,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    true);  // overlaps checking


  // Phan Solution and  Nobium
  auto Nb = nist->FindOrBuildMaterial("G4_Nb");
 
  G4double Nb_posZ = Havarpos.getZ()+HavarsizeZ*0.5+Nb_h*0.5;
  G4cout<<"====> On Z-axis, Zmin of Nb: "<<G4BestUnit(Nb_posZ - Nb_h*0.5,"Length")<<G4endl;
  G4ThreeVector Nb_pos(0,0,Nb_posZ);
  auto Nb_solid = BuildSolidUnionTwo(Nb_h,Nb_r);
  auto logicNb = new G4LogicalVolume(Nb_solid,  // its solid
                                          Nb,  // its material
                                          "Nblayer");  // its name
  SetStepLimit(logicNb);
  new G4PVPlacement(nullptr,  // no rotation
                    Nb_pos,  // at position
                    logicNb,  // its logical volume
                    "Nblayer",  // its name
                    fLogicWorld,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    true);  // overlaps checking
  
  // Phan chua chat long gay phan ung
  // Calculate the Z position to align the front surfaces
  // Both solids have their "front" at -0.5*h
  G4double zOffset = -0.5 * Nb_h + 0.5 * fTargetChamber_h;  
  G4ThreeVector posTarget(0, 0, zOffset);
  auto fTargetSolution =  DefineLiquidTargetMaterial();
  auto targetSolution_solid = BuildSolidUnionTwo(fTargetChamber_h,fTargetChamber_r);
  auto logicTargetSolution = new G4LogicalVolume(targetSolution_solid,  // its solid
                                          fTargetSolution,  // its material
                                          "Target");  // its name
  SetStepLimit(logicTargetSolution);
  new G4PVPlacement(nullptr,  // no rotation
                    posTarget,  // at position
                    logicTargetSolution,  // its logical volume
                    "Target",  // its name
                    logicNb,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    true);  // overlaps checking
  fScoringVolume = logicTargetSolution;

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4UnionSolid* DetectorConstruction::BuildSolidUnionTwo(G4double h, G4double r)
{
  // Create the cylindrical part
  G4Tubs* cyl = new G4Tubs("Cylinder",0,r,0.5*h,0,360*deg);
  // Create the hemispherical part (half-sphere)
  auto* sphere = new G4Sphere("Sphere",0,r,0,180*deg,0,180*deg);
  // Define rotation for the hemisphere to align with the cylinder's bottom
  G4RotationMatrix* Rot = new G4RotationMatrix;  // Rotates X and Z axes only
  Rot->rotateX(-90*deg);    
  // Translation to place the hemisphere at the end of the cylinder (z = -h/2 or h/2 depending on orientation)                 // Rotates 45 degrees
  G4ThreeVector zTrans(0, 0, h*0.5);
  // Combine the shapes into a single solid
  G4UnionSolid* unionShape = new G4UnionSolid("Cylinder+Sphere", cyl,sphere,Rot,zTrans);
  return unionShape;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Material* DetectorConstruction::DefineLiquidTargetMaterial() {
    G4NistManager* nist = G4NistManager::Instance();

    // 1. Define Elements and Isotopes
    G4double z, a, density;
    G4String name, symbol;
    G4int nComponents, nAtoms;

    
    // Target isotope: Ni-64
    G4Isotope* iso_Ni64 = new G4Isotope("Ni64", 28, 64, 63.9280*g/mole);
    
    // Common impurities in enriched Ni-64 (approximated based on commercial standards)
    G4Isotope* iso_Ni62 = new G4Isotope("Ni62", 28, 62, 61.9283*g/mole);
    G4Isotope* iso_Ni61 = new G4Isotope("Ni61", 28, 61, 60.9311*g/mole);
    G4Isotope* iso_Ni60 = new G4Isotope("Ni60", 28, 60, 59.9308*g/mole);
    G4Isotope* iso_Ni58 = new G4Isotope("Ni58", 28, 58, 57.9353*g/mole);

    // Create the Enriched Nickel element (99% Enrichment)
    G4Element* elEnrichedNi = new G4Element("EnrichedNi", "Ni", 5);
    elEnrichedNi->AddIsotope(iso_Ni64, 99.00*perCent); // Target Enrichment
    elEnrichedNi->AddIsotope(iso_Ni62,  0.40*perCent);
    elEnrichedNi->AddIsotope(iso_Ni61,  0.10*perCent);
    elEnrichedNi->AddIsotope(iso_Ni60,  0.35*perCent);
    elEnrichedNi->AddIsotope(iso_Ni58,  0.15*perCent);

    // Get other elements from NIST
    G4Element* elH = nist->FindOrBuildElement("H");
    G4Element* elN = nist->FindOrBuildElement("N");
    G4Element* elO = nist->FindOrBuildElement("O");

    // 2. Define Components of the solution
    
    // Nickel Nitrate: Ni(NO3)2
    G4Material* NiNO3_2 = new G4Material("NickelNitrate", density=2.05*g/cm3, nComponents=3);
    NiNO3_2->AddElement(elEnrichedNi, nAtoms=1);
    NiNO3_2->AddElement(elN, nAtoms=2);
    NiNO3_2->AddElement(elO, nAtoms=6);

    // Nitric Acid: HNO3 (Assume 1.51 g/cm3 for pure, but we use mass fraction later)
    G4Material* HNO3 = new G4Material("NitricAcid", density=1.51*g/cm3, nComponents=3);
    HNO3->AddElement(elH, nAtoms=1);
    HNO3->AddElement(elN, nAtoms=1);
    HNO3->AddElement(elO, nAtoms=3);

    // Water: H2O
    G4Material* H2O = nist->FindOrBuildMaterial("G4_WATER");

    // 3. Create the Final Liquid Target Solution
    // Target: 100mg Ni-64 in 2ml solution (~2200mg total)
    // Calculated Mass Fraction for Ni(NO3)2 is 13.35%
    G4double solutionDensity = 1.15*g/cm3; // Estimated density of the mixture
    G4Material* targetSolution = new G4Material("TargetSolution", solutionDensity, nComponents=3);
    
    targetSolution->AddMaterial(NiNO3_2, 13.35*perCent);   // Ni(NO3)2: 13.35% ; Hieu kiem tra
    targetSolution->AddMaterial(HNO3,    2.0*perCent);   // Typical low concentration
    targetSolution->AddMaterial(H2O,     84.65*perCent);  // Remaining is water
    return targetSolution;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Material* DetectorConstruction::DefineHavarMaterial() {
  G4NistManager* nist = G4NistManager::Instance();

  // 1. Fetch required elements from the NIST database
  G4Element* elCo = nist->FindOrBuildElement("Co");
  G4Element* elCr = nist->FindOrBuildElement("Cr");
  G4Element* elNi = nist->FindOrBuildElement("Ni");
  G4Element* elW  = nist->FindOrBuildElement("W");
  G4Element* elMo = nist->FindOrBuildElement("Mo");
  G4Element* elMn = nist->FindOrBuildElement("Mn");
  G4Element* elFe = nist->FindOrBuildElement("Fe");
  G4Element* elC  = nist->FindOrBuildElement("C");
  G4Element* elBe = nist->FindOrBuildElement("Be");

  // 2. Create the Havar Material
  // Standard density for Havar is 8.3 g/cm3
  G4double havarDensity = 8.3 * g/cm3;
  G4Material* havar = new G4Material("Havar", havarDensity, 9);

  // 3. Add elements according to the specified mass fractions
  havar->AddElement(elCo, 42.00 * perCent);
  havar->AddElement(elCr, 19.50 * perCent);
  havar->AddElement(elNi, 12.70 * perCent);
  havar->AddElement(elFe, 19.05 * perCent); // Calculated balance
  havar->AddElement(elW,   2.70 * perCent);
  havar->AddElement(elMo,  2.20 * perCent);
  havar->AddElement(elMn,  1.60 * perCent);
  havar->AddElement(elC,   0.20 * perCent);
  havar->AddElement(elBe,  0.05 * perCent);
  return havar;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Material* DetectorConstruction::DefineHeliumGas() {
  G4NistManager* nist = G4NistManager::Instance();
  
  // Define parameters for 2.0 bar Helium
  G4double pressure = 2.0 * bar;
  G4double temperature = 293.15 * kelvin; // 20 degrees Celsius
  G4double density = 0.328 * kg/m3;       // Calculated density at 2.0 bar

  // Create Material: "Helium_2bar"
  // We use G4_He as a base but redefine its physical state
  G4Material* he2bar = new G4Material("Helium_2bar", 
                                      density, 
                                      1,         // Number of components
                                      kStateGas, 
                                      temperature, 
                                      pressure);
  
  // Add Helium element from NIST
  G4Element* elHe = nist->FindOrBuildElement("He");
  he2bar->AddElement(elHe, 1.0);
  return he2bar;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetStepLimit(G4LogicalVolume* lv)
{
  if (fUsingStepLimit) {
    auto afStepLimit = new G4UserLimits(fStepMax);
    lv->SetUserLimits(afStepLimit);
  }
}