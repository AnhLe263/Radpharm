#ifndef TrackInformation_h
#define TrackInformation_h 1

#include "G4VUserTrackInformation.hh"
#include "globals.hh"
#include "G4Allocator.hh"
#include <vector>

class TrackInformation : public G4VUserTrackInformation
{
public:

  TrackInformation();
  virtual ~TrackInformation();

  TrackInformation(const TrackInformation&);

  inline void* operator new(size_t);
  inline void  operator delete(void*);

public:

  // flag
  G4bool isNuclearProduct = false;

  // avoid double counting
  G4bool alreadyRecorded = false;

  // reaction information
  G4int projectilePDG = 0;
  G4int targetPDG     = 0;

  std::vector<G4int> productPDGs;

  G4String creatorProcess;

  G4int sourceVolumeID = -1;
};

extern G4ThreadLocal
G4Allocator<TrackInformation>* TrackInformationAllocator;

inline void* TrackInformation::operator new(size_t)
{
  if (!TrackInformationAllocator)
  {
    TrackInformationAllocator =
      new G4Allocator<TrackInformation>;
  }

  return (void*)TrackInformationAllocator->MallocSingle();
}

inline void TrackInformation::operator delete(void* aTrackInfo)
{
  TrackInformationAllocator->FreeSingle(
    (TrackInformation*)aTrackInfo);
}

#endif
