#ifndef __IFF_SYSTEM_H__
#define __IFF_SYSTEM_H__

#include "Aircraft.h"
#include <vector>
#include <map>

// ============================================================================
// IFF (Identification Friend or Foe) System
// Automatic interrogation of radar contacts using Mode 4
// ============================================================================

struct IFFInterrogation {
    int trackId;
    float timeRemaining;    // Seconds until result
    Aircraft* aircraft;
};

class IFFSystem {
public:
    IFFSystem();
    ~IFFSystem();

    // Begin interrogation of a new contact
    void interrogate(Aircraft* aircraft);

    // Update all pending interrogations
    void update(float dt);

    // Check if aircraft is being interrogated
    bool isInterrogating(int trackId) const;

    // Set error rate (from game level config)
    void setErrorRate(float rate) { errorRate_ = rate; }

private:
    std::vector<IFFInterrogation> pendingInterrogations_;
    float errorRate_;   // Probability of incorrect IFF reading

    // Determine IFF result with possible errors
    IFFStatus determineResult(Aircraft* aircraft);
};

#endif // __IFF_SYSTEM_H__
