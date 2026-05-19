#ifndef ReactionData_h
#define ReactionData_h 1

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "globals.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"

struct ReactionKey {

  int projectilePDG;
  int targetPDG;

  std::vector<int> productPDGs;

  bool operator==(const ReactionKey& other) const
  {
    return projectilePDG == other.projectilePDG &&
           targetPDG == other.targetPDG &&
           productPDGs == other.productPDGs;
  }
};

struct ReactionKeyHash {

  std::size_t operator()(const ReactionKey& k) const
  {
    std::size_t h = std::hash<int>()(k.projectilePDG);

    h ^= std::hash<int>()(k.targetPDG)
         + 0x9e3779b9 + (h<<6) + (h>>2);

    for (auto p : k.productPDGs) {
      h ^= std::hash<int>()(p)
           + 0x9e3779b9 + (h<<6) + (h>>2);
    }

    return h;
  }
};

struct ReactionData {
  uint64_t count = 0;
};

using ReactionTable = std::unordered_map<ReactionKey,
                                         ReactionData,
                                         ReactionKeyHash>;


inline G4String PDGToName(int pdg)
{
  auto particle =
    G4ParticleTable::GetParticleTable()->FindParticle(pdg);

  if (particle)
    return particle->GetParticleName();

  if (pdg > 1000000000) {

    int Z = (pdg / 10000) % 1000;
    int A = (pdg / 10) % 1000;

    auto ion =
      G4IonTable::GetIonTable()->GetIon(Z, A, 0.0);

    if (ion)
      return ion->GetParticleName();
  }

  return "unknown";
}

inline G4String ReactionToString(const ReactionKey& key)
{
  std::ostringstream oss;

  oss << PDGToName(key.projectilePDG)
      << " + "
      << PDGToName(key.targetPDG)
      << " -> ";

  for (size_t i = 0; i < key.productPDGs.size(); ++i) {

    oss << PDGToName(key.productPDGs[i]);

    if (i + 1 < key.productPDGs.size())
      oss << " + ";
  }

  return oss.str();
}

#endif