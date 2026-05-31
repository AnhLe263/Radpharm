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
#include "G4String.hh"


DetectorConstruction::DetectorConstruction():G4VUserDetectorConstruction()
{
  fMessenger = new G4GenericMessenger(this,"/target/","");
  fMessenger->DeclarePropertyWithUnit("thickness","mm",targetSizeZ,"");
  fMessenger->DeclareProperty("usingStepMax",fUsingStepLimit,"");
  fMessenger->DeclarePropertyWithUnit("stepMax","mm",fStepMax,"");
  fMessenger->DeclareProperty("CSVFileNameForMat",fCSVFileName,"");
  fMessenger->DeclareProperty("HNO3Molarity",fTargetHNO3Molarity,"");
  fMessenger->DeclareProperty("beamEntranceWindowMaterial",fBeamEntranceWindowMatName,"");
  fMessenger->DeclarePropertyWithUnit("beamEntranceWindowThickness","mm",fBeamEntranceWindowThickness,"");
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
  G4bool checkOverlaps = true;

  //------------------------------------------
  // Materials
  //------------------------------------------
  auto nist = G4NistManager::Instance();
  auto liquidMat = CreateTargetMaterialFromCSV();
  G4Material* beamEntranceWindowMat = nullptr;
  G4StrUtil::to_lower(fBeamEntranceWindowMatName);
  if ( fBeamEntranceWindowMatName == "havar") {
    beamEntranceWindowMat = DefineHavarMaterial();
  }
  if ( fBeamEntranceWindowMatName == "ti") {
    beamEntranceWindowMat = nist->FindOrBuildMaterial("G4_Ti");
  }
 
  auto heliumMat = DefineHeliumGas();

  

  auto nbMat =
  nist->FindOrBuildMaterial("G4_Nb");

  //------------------------------------------
  // Geometry
  //------------------------------------------

  auto geom =
  BuildConicalLiquidTarget();

  //------------------------------------------
  // Dimensions
  //------------------------------------------

  const G4double heThickness =
      2.0*mm;

  const G4double targetWindowThickness =
      35.0*um;

  //------------------------------------------
  // Beam Entrance Window
  //------------------------------------------

  auto beamWindowSolid =
  new G4Tubs(
      "BeamWindow",
      0.0,
      geom.Rfront+ geom.NbThickness,
      fBeamEntranceWindowThickness/2.0,
      0.*deg,
      360.*deg
  );

  auto beamWindowLV =
  new G4LogicalVolume(
      beamWindowSolid,
      beamEntranceWindowMat,
      "BeamWindowLV"
  );

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(
          0,
          0,
          0
      ),
      beamWindowLV,
      "BeamWindow",
      fLogicWorld,
      false,
      0,
      checkOverlaps
  );

  //------------------------------------------
  // He cavity
  //------------------------------------------

  auto heSolid =
  new G4Tubs(
      "HeCavity",
      0.0,
      geom.Rfront+ geom.NbThickness,
      heThickness/2.0,
      0.*deg,
      360.*deg
  );

  auto heLV =
  new G4LogicalVolume(
      heSolid,
      heliumMat,
      "HeCavityLV"
  );

  G4double zHe =
      fBeamEntranceWindowThickness/2.0
    + heThickness/2.0;

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(
          0,
          0,
          zHe
      ),
      heLV,
      "HeCavity",
      fLogicWorld,
      false,
      0,
      checkOverlaps
  );

  //------------------------------------------
  // Target Window (Nb)
  //------------------------------------------

  auto targetWindowSolid =
  new G4Tubs(
      "TargetWindow",
      0.0,
      geom.Rfront + geom.NbThickness,
      targetWindowThickness/2.0,
      0.*deg,
      360.*deg
  );

  auto targetWindowLV =
  new G4LogicalVolume(
      targetWindowSolid,
      nbMat,
      "TargetWindowLV"
  );

  G4double zTargetWindow =
      fBeamEntranceWindowThickness/2.0
    + heThickness
    + targetWindowThickness/2.0;

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(
          0,
          0,
          zTargetWindow
      ),
      targetWindowLV,
      "TargetWindow",
      fLogicWorld,
      false,
      0,
      checkOverlaps
  );

  //------------------------------------------
  // Nb outer body
  //------------------------------------------

  auto nbOuterLV =
  new G4LogicalVolume(
      geom.nbOuterSolid,
      nbMat,
      "NbOuterLV"
  );

  G4double zNbOuter =
      fBeamEntranceWindowThickness/2.0
    + heThickness
    + targetWindowThickness
    + geom.coneLength/2.0;

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(
          0,
          0,
          zNbOuter
      ),
      nbOuterLV,
      "NbOuter",
      fLogicWorld,
      false,
      0,
      checkOverlaps
  );

  //------------------------------------------
  // Liquid target
  //------------------------------------------

  auto liquidLV =
  new G4LogicalVolume(
      geom.liquidSolid,
      liquidMat,
      "LiquidTargetLV"
  );

  //------------------------------------------
  // Liquid is daughter of NbOuter
  //------------------------------------------

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(
          0,
          0,
          0
      ),
      liquidLV,
      "LiquidTarget",
      nbOuterLV,
      false,
      0,
      checkOverlaps
  );

  //------------------------------------------
  // Scoring volume
  //------------------------------------------

  fScoringVolume = liquidLV;

  SetStepLimit(liquidLV);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ConicalTargetGeometry
DetectorConstruction::BuildConicalLiquidTarget()
{
    ConicalTargetGeometry geom;

    //------------------------------------------
    // Optimized geometry
    //------------------------------------------

    geom.Rfront = 7.80*mm;
    geom.Rback  = 4.00*mm;

    geom.coneLength   = 16.0*mm;
    geom.sphereRadius = 4.0*mm;

    geom.totalLength  = 20.0*mm; // Total langth of liquid solid

    geom.NbThickness  = 1.0*mm;

    //------------------------------------------
    // Liquid cone
    //------------------------------------------

    auto liquidCone =
    new G4Cons(
        "LiquidCone",
        0.0,
        geom.Rfront,
        0.0,
        geom.Rback,
        geom.coneLength/2.0,
        0.*deg,
        360.*deg
    );

    //------------------------------------------
    // Hemisphere (+Z side)
    //------------------------------------------

    auto liquidSphere =
    new G4Sphere(
        "LiquidSphere",
        0.0,
        geom.sphereRadius,
        0.*deg,
        360.*deg,
        0.*deg,
        90.*deg
    );

    //------------------------------------------
    // Liquid target
    //------------------------------------------

    geom.liquidSolid =
    new G4UnionSolid(
        "LiquidTarget",
        liquidCone,
        liquidSphere,
        nullptr,
        G4ThreeVector(
            0.0,
            0.0,
            geom.coneLength/2.0
        )
    );

    //------------------------------------------
    // Nb outer cone
    //------------------------------------------

    auto nbCone =
    new G4Cons(
        "NbCone",
        0.0,
        geom.Rfront + geom.NbThickness,
        0.0,
        geom.Rback  + geom.NbThickness,
        geom.coneLength/2.0,
        0.*deg,
        360.*deg
    );

    //------------------------------------------
    // Nb hemisphere
    //------------------------------------------

    auto nbSphere =
    new G4Sphere(
        "NbSphere",
        0.0,
        geom.sphereRadius + geom.NbThickness,
        0.*deg,
        360.*deg,
        0.*deg,
        90.*deg
    );

    //------------------------------------------
    // Nb outer body
    //------------------------------------------

    geom.nbOuterSolid =
    new G4UnionSolid(
        "NbOuter",
        nbCone,
        nbSphere,
        nullptr,
        G4ThreeVector(
            0.0,
            0.0,
            geom.coneLength/2.0
        )
    );

    return geom;
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
  // 1. Define base elements and enriched isotopes
  G4NistManager* nist = G4NistManager::Instance();
  G4Element* elH = nist->FindOrBuildElement("H");
  G4Element* elN = nist->FindOrBuildElement("N");
  G4Element* elO = nist->FindOrBuildElement("O");

  // Define enriched 64Ni isotope (Molar mass: 63.928 g/mol)
  G4Isotope* isoNi64 = new G4Isotope("Ni64", 28, 64, 63.928*g/mole);
  G4Element* elNi64 = new G4Element("enrichedNi64", "64Ni", 1);
  elNi64->AddIsotope(isoNi64, 100.*perCent);

  // 2. Variables for material properties
  G4double density;
  G4double fracNi, fracN, fracO, fracH;
  G4String matName;

  // 3. Material selection based on HNO3 molarity (Calculated for 100 mg Ni in 2 mL)
  if (fConcentration == "0M") {
      matName = "Ni64Sol_0M";
      density = 1.1184 * g/cm3;
      fracNi  = 4.471 * perCent;
      fracN   = 1.959 * perCent;
      fracO   = 83.851 * perCent;
      fracH   = 9.719 * perCent;
  } 
  else if (fConcentration == "0.005M") {
      matName = "Ni64Sol_0.005M";
      density = 1.1187 * g/cm3;
      fracNi  = 4.470 * perCent;
      fracN   = 1.965 * perCent;
      fracO   = 83.849 * perCent;
      fracH   = 9.716 * perCent;
  }
  else if (fConcentration == "0.01M") {
      matName = "Ni64Sol_0.01M";
      density = 1.1190 * g/cm3;
      fracNi  = 4.469 * perCent;
      fracN   = 1.972 * perCent;
      fracO   = 83.846 * perCent;
      fracH   = 9.713 * perCent;
  }
  else if (fConcentration == "0.05M") {
      // Recommended concentration for the simulation
      matName = "Ni64Sol_0.05M";
      density = 1.1213 * g/cm3;
      fracNi  = 4.459 * perCent;
      fracN   = 2.022 * perCent;
      fracO   = 83.827 * perCent;
      fracH   = 9.692 * perCent;
  } 
  else if (fConcentration == "0.1M") {
      matName = "Ni64Sol_0.1M";
      density = 1.1241 * g/cm3;
      fracNi  = 4.448 * perCent;
      fracN   = 2.084 * perCent;
      fracO   = 83.803 * perCent;
      fracH   = 9.665 * perCent;
  } 
  else if (fConcentration == "0.5M") {
      matName = "Ni64Sol_0.5M";
      density = 1.1472 * g/cm3;
      fracNi  = 4.358 * perCent;
      fracN   = 2.574 * perCent;
      fracO   = 83.618 * perCent;
      fracH   = 9.450 * perCent;
  }
  else if (fConcentration == "1M") {
      matName = "Ni64Sol_1M";
      density = 1.1764 * g/cm3;
      fracNi  = 4.250 * perCent;
      fracN   = 3.155 * perCent;
      fracO   = 83.395 * perCent;
      fracH   = 9.200 * perCent;
  }
  else {
      G4cerr << "Invalid concentration! Defaulting to 0.05M" << G4endl;
      fConcentration = "0.05M";
      return DefineLiquidTargetMaterial();
  }

  // 4. Create the final G4Material object
  G4Material* solution = new G4Material(matName, density, 4);
  solution->AddElement(elNi64, fracNi);
  solution->AddElement(elN,    fracN);
  solution->AddElement(elO,    fracO);
  solution->AddElement(elH,    fracH);

  return solution;
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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


G4Material* DetectorConstruction::CreateTargetMaterialFromCSV()
{
    // Path to your actual CSV data file
    std::ifstream file(fCSVFileName);
    
    if (!file.is_open()) {
        G4Exception("DetectorConstruction::CreateTargetMaterialFromCSV", "FileNotOpen",
                    FatalException, ("Cannot open CSV data file: " + fCSVFileName).c_str());
    }
    
    std::string line;
    std::getline(file, line); // Skip the header row of the CSV file
    
    G4bool found = false;
    G4double minDifference = 9999.0;
    
    // Variables to store the retrieved values from the best-matching row
    G4double resMolarity = 0.0;
    G4double resDensity = 0.0;
    G4double wt_N = 0.0, wt_O = 0.0, wt_H = 0.0;
    G4double wt_Ni64 = 0.0, wt_Ni58 = 0.0, wt_Ni60 = 0.0, wt_Ni61 = 0.0, wt_Ni62 = 0.0;
    
    // Read and scan the file to find the closest molarity matching the target
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        std::vector<G4double> rowValues;
        
        while (std::getline(ss, cell, ',')) {
            rowValues.push_back(std::stod(cell));
        }
        
        if (rowValues.size() < 14) continue; // Skip corrupted or incomplete rows
        
        G4double molarity = rowValues[0];
        G4double diff = std::abs(molarity - fTargetHNO3Molarity);
        
        // Select the row with the minimum difference (maximum allowed tolerance is 0.05 M)
        if (diff < minDifference && diff < 0.05) {
            minDifference = diff;
            resMolarity = rowValues[0];
            resDensity  = rowValues[1];
            wt_N        = rowValues[6];
            wt_O        = rowValues[7];
            wt_H        = rowValues[8];
            wt_Ni64     = rowValues[9];
            wt_Ni58     = rowValues[10];
            wt_Ni60     = rowValues[11];
            wt_Ni61     = rowValues[12];
            wt_Ni62     = rowValues[13];
            found = true;
        }
    }
    file.close();
    
    if (!found) {
        G4Exception("DetectorConstruction::CreateTargetMaterialFromCSV", "MolarityNotFound",
                    FatalException, "The requested HNO3 molarity was not found in the CSV file!");
    }
    
    G4cout << "---> [Geant4] Configuration loaded successfully for molarity: " << resMolarity << " M" << G4endl;
    G4cout << "---> [Geant4] Solution density: " << resDensity << " g/cm3" << G4endl;

    // Retrieve base elements from the NIST manager for non-metals
    G4NistManager* nist = G4NistManager::Instance();
    G4Element* elH = nist->FindOrBuildElement("H");
    G4Element* elN = nist->FindOrBuildElement("N");
    G4Element* elO = nist->FindOrBuildElement("O");
    
    // Define the single stable isotopes of Nickel with their respective atomic masses
    G4Isotope* isoNi58 = new G4Isotope("isoNi58", 28, 58, 57.9353*g/mole);
    G4Isotope* isoNi60 = new G4Isotope("isoNi60", 28, 60, 59.9308*g/mole);
    G4Isotope* isoNi61 = new G4Isotope("isoNi61", 28, 61, 60.9311*g/mole);
    G4Isotope* isoNi62 = new G4Isotope("isoNi62", 28, 62, 61.9283*g/mole);
    G4Isotope* isoNi64 = new G4Isotope("isoNi64", 28, 64, 63.9280*g/mole);
    
    // Construct a custom element for Nickel from the individual isotopes
    G4Element* elCustomNi = new G4Element("CustomNi", "Ni", 5);
    
    // Calculate the relative abundance fractions among the Ni isotopes,
    // as the CSV values represent the weight percentage relative to the entire solution.
    G4double totalNiWeight = wt_Ni64 + wt_Ni58 + wt_Ni60 + wt_Ni61 + wt_Ni62;
    
    if (totalNiWeight > 0.0) {
        elCustomNi->AddIsotope(isoNi58, wt_Ni58 / totalNiWeight);
        elCustomNi->AddIsotope(isoNi60, wt_Ni60 / totalNiWeight);
        elCustomNi->AddIsotope(isoNi61, wt_Ni61 / totalNiWeight);
        elCustomNi->AddIsotope(isoNi62, wt_Ni62 / totalNiWeight);
        elCustomNi->AddIsotope(isoNi64, wt_Ni64 / totalNiWeight);
    } else {
        // Fallback default if the target solution contains zero nickel concentration
        elCustomNi->AddIsotope(isoNi58, 100.0*perCent); 
    }

    // Build the final dynamic target material composite (composed of H, N, O, and custom Ni)
    std::string matName = "Target_Sol_" + std::to_string(resMolarity) + "M";
    G4Material* targetMaterial = new G4Material(matName, resDensity * g/cm3, 4);
    
    // Divide by 100.0 since data values in the CSV file are given in weight percentages (0-100%)
    targetMaterial->AddElement(elH,        wt_H / 100.0);
    targetMaterial->AddElement(elN,        wt_N / 100.0);
    targetMaterial->AddElement(elO,        wt_O / 100.0);
    targetMaterial->AddElement(elCustomNi, totalNiWeight / 100.0);
    
    return targetMaterial;
}