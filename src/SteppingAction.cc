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
/// \file SteppingAction.cc
/// \brief Implementation of the B1::SteppingAction class

#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"

#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4AnalysisManager.hh"
#include "G4HadronicProcess.hh"
#include "G4GenericMessenger.hh"
#include "G4Proton.hh"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(EventAction* eventAction) : fEventAction(eventAction) 
{
  fMessenger = new G4GenericMessenger(this, "/myscoring/", "Custom control commands");
  fMessenger->DeclareProperty("scoringenergyout", fScoreEnergyOut, "Enable scoring energy");
}

SteppingAction::~SteppingAction()
{
  delete fMessenger;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(const G4Step* step)
{
   // collect energy deposited in this step
  G4double edepStep = step->GetTotalEnergyDeposit();
  if (edepStep <= 0) return;

  fEventAction->AddEdep(edepStep);
  G4Track* mothertrack = step->GetTrack();

  // get volume of the current step
  auto volume =
    step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();

  auto volID = GetVoulumeID(volume);

  if (fScoreEnergyOut && mothertrack->GetDefinition() == G4Proton::Definition() && step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) {
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleIColumn(1,0,volID);
    analysisManager->FillNtupleDColumn(1,1,step->GetPostStepPoint()->GetKineticEnergy());
    analysisManager->AddNtupleRow(1);
  }
 
  const G4VProcess* process = step->GetPostStepPoint()->GetProcessDefinedStep();
  if (process) {
    G4ProcessType type = process->GetProcessType();
    if (type == fHadronic) {
      G4String procName = process->GetProcessName();
      //G4StrUtil::to_lower(procName);
      if (G4StrUtil::contains(procName, "Inelastic")) {
        const auto* secondaries = step->GetSecondaryInCurrentStep();
        G4int Z = 999, A = 999;
        if (secondaries->size()>0) {
          auto hadProcess = const_cast<G4HadronicProcess*>(
                static_cast<const G4HadronicProcess*>(process));
          const G4Nucleus* nucleus = hadProcess->GetTargetNucleus();
          if (nucleus) {
            Z = nucleus->GetZ_asInt();
            A = nucleus->GetA_asInt();
          }
          G4String tarInfo = std::to_string(Z) + "-" +std::to_string(A);
          auto analysisManager = G4AnalysisManager::Instance();
          G4bool transmutationOccurred = true;
          G4bool DoesItProduceSameTrack = false;
          G4bool DoesItProduceSameAsTarget = false;
          for (const auto* track : *secondaries)
          {   
            auto secZ = track->GetDefinition()->GetAtomicNumber();
            auto secA = track->GetDefinition()->GetAtomicMass();
            if (track->GetDefinition() == mothertrack->GetParticleDefinition()) DoesItProduceSameTrack = true;
            else if (secZ == Z && secA == A) DoesItProduceSameAsTarget = true;
            
          }
          if ( DoesItProduceSameAsTarget && DoesItProduceSameTrack) transmutationOccurred = false;
          if (transmutationOccurred) {
            G4ThreeVector interactionPos = step->GetPostStepPoint()->GetPosition();
            G4double x = interactionPos.x();
            G4double y = interactionPos.y();
            G4double z = interactionPos.z();
            for (const auto* track : *secondaries)
          {   
            G4String name = track->GetDefinition()->GetParticleName();
            analysisManager->FillNtupleSColumn(0,0, name);
            analysisManager->FillNtupleIColumn(0,1,volID);
            analysisManager->FillNtupleSColumn(0,2, mothertrack->GetParticleDefinition()->GetParticleName());
            analysisManager->FillNtupleSColumn(0,3, tarInfo);
            analysisManager->FillNtupleDColumn(0,4,x);
            analysisManager->FillNtupleDColumn(0,5,y);
            analysisManager->FillNtupleDColumn(0,6,z);
            analysisManager->AddNtupleRow(0);
          }
          }
          
        }
      }
    }
  }
  

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


G4int SteppingAction::GetVoulumeID(G4VPhysicalVolume *vol)
{
  G4int ID = -1;
  auto volName = vol->GetName();
  if (volName == "World") ID = 0;
  if (volName == "Tilayer") ID = 1;
  if (volName == "Helayer") ID = 2;
  if (volName == "Havarlayer") ID = 3;
  if (volName == "Nblayer") ID = 4;
  if (volName == "Target") ID = 5;
  return ID;
}
