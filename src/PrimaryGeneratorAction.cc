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
/// \file PrimaryGeneratorAction.cc
/// \brief Implementation of the B1::PrimaryGeneratorAction class

#include "PrimaryGeneratorAction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"



//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  fParticleGun = new G4GeneralParticleSource();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  /*
  G4SPSPosDistribution* posDist = fParticleGun->GetCurrentSource()->GetPosDist();
  posDist->SetPosDisType("Beam"); 
  posDist->SetPosDisShape("Circle");
  
  // Use Sigma instead of Radius to create a Gaussian distribution
  // For a target with r = 8.5 mm, a sigma of 3.0 mm ensures 
  // most particles hit the center and stay within the chamber.
  posDist->SetBeamSigmaInR(3.0 * mm); 
  
  posDist->SetCentreCoords(G4ThreeVector(0., 0., -5. * mm));

  G4SPSAngDistribution* angDist = fParticleGun->GetCurrentSource()->GetAngDist();
  angDist->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));

  G4SPSEneDistribution* eneDist = fParticleGun->GetCurrentSource()->GetEneDist();
  eneDist->SetEnergyDisType("Mono");
  eneDist->SetMonoEnergy(15.0 * MeV);
  */
  for (G4int i = 0; i < fnParticlePerEvent; i++) {
      fParticleGun->GeneratePrimaryVertex(event);
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

