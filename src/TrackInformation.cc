#include "TrackInformation.hh"

G4ThreadLocal
G4Allocator<TrackInformation>*
TrackInformationAllocator = 0;

TrackInformation::TrackInformation()
 : G4VUserTrackInformation()
{
}

TrackInformation::~TrackInformation()
{
}

TrackInformation::TrackInformation(
  const TrackInformation& right)
 : G4VUserTrackInformation()
{
  isNuclearProduct = right.isNuclearProduct;

  alreadyRecorded = right.alreadyRecorded;

  projectilePDG = right.projectilePDG;
  targetPDG     = right.targetPDG;

  productPDGs = right.productPDGs;

  creatorProcess = right.creatorProcess;

  sourceVolumeID = right.sourceVolumeID;
}
