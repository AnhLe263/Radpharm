//
// ********************************************************************
// * License and Disclaimer                                           *
// ********************************************************************
//
/// \file SteppingAction.cc
/// \brief Implementation of the B1::SteppingAction class

#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "TrackInformation.hh"

#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4AnalysisManager.hh"
#include "G4HadronicProcess.hh"
#include "G4GenericMessenger.hh"
#include "G4Proton.hh"
#include "G4SystemOfUnits.hh"

#include "RunAction.hh"

#include "G4Neutron.hh"
#include "G4Gamma.hh"
#include "G4IonTable.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(EventAction* eventAction)
 : fEventAction(eventAction)
{
  fMessenger =
    new G4GenericMessenger(
      this,
      "/myscoring/",
      "Custom control commands");

  fMessenger->DeclareProperty(
    "scoringenergyout",
    fScoreEnergyOut,
    "Enable scoring energy");

  fMessenger->DeclareProperty(
    "scoringEdep",
    fScoreEnergyDeposit,
    "Enable scoring edep");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::~SteppingAction()
{
  delete fMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(
  const G4Step* step)
{
  // =====================================================
  // Energy deposition
  // =====================================================

  G4double edepStep =
    step->GetTotalEnergyDeposit();

  if (edepStep > 0)
  {
    fEventAction->AddEdep(edepStep);
  }

  G4Track* mothertrack =
    step->GetTrack();

  // =====================================================
  // Current volume
  // =====================================================

  auto volume =
    step->GetPreStepPoint()
        ->GetTouchableHandle()
        ->GetVolume();

  const G4int volID =
    GetVoulumeID(volume);

  auto analysisManager =
    G4AnalysisManager::Instance();

  // =====================================================
  // Energy deposition scoring
  // =====================================================

  if (fScoreEnergyDeposit &&
      mothertrack->GetDefinition()
        == G4Proton::Definition())
  {
    G4ThreeVector P1 =
      step->GetPreStepPoint()->GetPosition();

    G4ThreeVector P2 =
      step->GetPostStepPoint()->GetPosition();

    G4ThreeVector point =
      P1 + G4UniformRand() * (P2 - P1);

    G4double z = point.z();
    G4double x = point.x();

    analysisManager->FillNtupleIColumn(
      2, 0, volID);

    analysisManager->FillNtupleDColumn(
      2, 1, edepStep / keV);

    analysisManager->FillNtupleDColumn(
      2, 2, x);

    analysisManager->FillNtupleDColumn(
      2, 3, z);

    analysisManager->AddNtupleRow(2);
  }

  // =====================================================
  // Outgoing proton scoring
  // =====================================================

  if (fScoreEnergyOut &&
      mothertrack->GetDefinition()
        == G4Proton::Definition() &&
      step->GetPostStepPoint()->GetStepStatus()
        == fGeomBoundary)
  {
    analysisManager->FillNtupleIColumn(
      1, 0, volID);

    analysisManager->FillNtupleDColumn(
      1, 1,
      step->GetPostStepPoint()
          ->GetKineticEnergy());

    analysisManager->AddNtupleRow(1);
  }

  // =====================================================
  // Nuclear reactions
  // =====================================================

  const G4VProcess* process =
    step->GetPostStepPoint()
        ->GetProcessDefinedStep();

  if (process)
  {
    if (process->GetProcessType() == fHadronic)
    {
      G4String procName =
        process->GetProcessName();

      if (G4StrUtil::contains(
            procName,
            "Inelastic"))
      {
        auto hadProcess =
          const_cast<G4HadronicProcess*>(
            static_cast<
              const G4HadronicProcess*>(
                process));

        const G4Nucleus* nucleus =
          hadProcess->GetTargetNucleus();

        if (nucleus)
        {
          int targetZ =
            nucleus->GetZ_asInt();

          int targetA =
            nucleus->GetA_asInt();

          auto ion =
            G4IonTable::GetIonTable()
              ->GetIon(
                targetZ,
                targetA,
                0.0);

          if (ion)
          {
            int targetPDG =
              ion->GetPDGEncoding();

            int projectilePDG =
              mothertrack->GetDefinition()
                ->GetPDGEncoding();

            const auto* secondaries =
              step->GetSecondaryInCurrentStep();

            if (secondaries &&
                !secondaries->empty())
            {
              
              // ===========================================
              // Check true transmutation
              // ===========================================

              G4bool transmutationOccurred = true;

              G4bool DoesItProduceSameTrack = false;
              G4bool DoesItProduceSameAsTarget = false;

              for (const auto* track : *secondaries)
              {
                auto secZ =
                  track->GetDefinition()->GetAtomicNumber();

                auto secA =
                  track->GetDefinition()->GetAtomicMass();

                if (track->GetDefinition() ==
                    mothertrack->GetParticleDefinition())
                {
                  DoesItProduceSameTrack = true;
                }
                else if (secZ == targetZ &&
                        secA == targetA)
                {
                  DoesItProduceSameAsTarget = true;
                }
              }

              if (DoesItProduceSameAsTarget &&
                  DoesItProduceSameTrack)
              {
                transmutationOccurred = false;
              }

              // ===========================================
              // Only real transmutation reactions
              // ===========================================

              if (transmutationOccurred)
              {
                std::vector<int> productPDGs;

                for (const auto* track :
                    *secondaries)
                {
                  auto def =
                    track->GetDefinition();

                  int pdg =
                    def->GetPDGEncoding();

                  productPDGs.push_back(pdg);
                }

                std::sort(
                  productPDGs.begin(),
                  productPDGs.end());

                ReactionKey key;

                key.projectilePDG =
                  projectilePDG;

                key.targetPDG =
                  targetPDG;

                key.productPDGs =
                  productPDGs;

                // =======================================
                // Direct reaction in Target
                // =======================================

                if (volID == 5)
                {
                  RunAction::AddReaction(key);

                  for (auto secTrack :
                      *secondaries)
                  {
                    auto info =
                      new TrackInformation();

                    info->alreadyRecorded = true;

                    secTrack->SetUserInformation(
                      info);
                  }
                }

                // =======================================
                // Attach TrackInformation
                // =======================================

                if (volID == 3 ||
                    volID == 4)
                {
                  for (auto secTrack :
                      *secondaries)
                  {
                    auto pd =
                      secTrack->GetDefinition();

                    // only residual nuclei
                    if (!pd->IsGeneralIon())
                      continue;

                    if (secTrack
                          ->GetUserInformation())
                      continue;

                    auto info =
                      new TrackInformation();

                    info->isNuclearProduct = true;

                    info->projectilePDG =
                      projectilePDG;

                    info->targetPDG =
                      targetPDG;

                    info->productPDGs =
                      productPDGs;

                    info->creatorProcess =
                      process->GetProcessName();

                    info->sourceVolumeID =
                      volID;

                    secTrack->SetUserInformation(
                      info);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // =====================================================
  // Detect nuclear products entering Target
  // =====================================================

  auto preVolume =
    step->GetPreStepPoint()
        ->GetTouchableHandle()
        ->GetVolume();

  auto postVolume =
    step->GetPostStepPoint()
        ->GetTouchableHandle()
        ->GetVolume();

  if (preVolume && postVolume)
  {
    G4int preVolID =
      GetVoulumeID(preVolume);

    G4int postVolID =
      GetVoulumeID(postVolume);

    if (preVolID != 5 &&
        postVolID == 5 &&
        step->GetPostStepPoint()
          ->GetStepStatus()
            == fGeomBoundary)
    {
      auto track =
        step->GetTrack();

      auto info =
        dynamic_cast<
          TrackInformation*>(
            track->GetUserInformation());

      if (info &&
          info->isNuclearProduct &&
          !info->alreadyRecorded)
      {
        ReactionKey key;

        key.projectilePDG =
          info->projectilePDG;

        key.targetPDG =
          info->targetPDG;

        key.productPDGs =
          info->productPDGs;

        std::sort(
          key.productPDGs.begin(),
          key.productPDGs.end());

        RunAction::AddReaction(key);

        info->alreadyRecorded = true;
      }
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4int SteppingAction::GetVoulumeID(
  G4VPhysicalVolume *vol)
{
  G4int ID = -1;

  auto volName = vol->GetName();

  if (volName == "World")
    ID = 0;

  if (volName == "Tilayer")
    ID = 1;

  if (volName == "Helayer")
    ID = 2;

  if (volName == "Havarlayer")
    ID = 3;

  if (volName == "Nblayer")
    ID = 4;

  if (volName == "Target")
    ID = 5;

  return ID;
}