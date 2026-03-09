#include "BattalionHQ.h"
#include <cstdio>

BattalionHQ::BattalionHQ()
    : designation_("MINDER-HQ")
    , status_(HQStatus::OPERATIONAL)
    , isRelocating_(false)
    , relocateTimeRemaining_(0.0f)
    , radarOnline_(false)
    , commsOnline_(false)
{
    position_.range = 0.0f;
    position_.azimuth = 0.0f;
    relocateDestination_.range = 0.0f;
    relocateDestination_.azimuth = 0.0f;
}

BattalionHQ::~BattalionHQ() {}

void BattalionHQ::init(float posRange, float posAzimuth)
{
    position_.range = posRange;
    position_.azimuth = posAzimuth;
    status_ = HQStatus::OPERATIONAL;
    isRelocating_ = false;
    relocateTimeRemaining_ = 0.0f;
    radarOnline_ = true;
    commsOnline_ = true;
}

void BattalionHQ::update(float dt)
{
    if (!isRelocating_) return;

    relocateTimeRemaining_ -= dt;

    if (relocateTimeRemaining_ <= 0.0f) {
        // Relocation complete — fully operational at new position
        position_ = relocateDestination_;
        isRelocating_ = false;
        relocateTimeRemaining_ = 0.0f;
        status_ = HQStatus::OPERATIONAL;
        radarOnline_ = true;
        commsOnline_ = true;
    } else if (relocateTimeRemaining_ <= SETUP_TIME) {
        // Setting up at new position — radar coming online first
        status_ = HQStatus::SETTING_UP;
        // Radar comes online in last 15 seconds of setup
        radarOnline_ = (relocateTimeRemaining_ <= SETUP_TIME * 0.5f);
        commsOnline_ = false;
    } else {
        // Still moving or tearing down
        status_ = HQStatus::RELOCATING;
        radarOnline_ = false;
        commsOnline_ = false;
    }
}

bool BattalionHQ::relocate(float newRange, float newAzimuth)
{
    if (isRelocating_) return false;
    if (status_ == HQStatus::SETTING_UP) return false;

    relocateDestination_.range = newRange;
    relocateDestination_.azimuth = newAzimuth;
    isRelocating_ = true;
    relocateTimeRemaining_ = TOTAL_RELOCATE_TIME;
    status_ = HQStatus::RELOCATING;
    radarOnline_ = false;
    commsOnline_ = false;

    return true;
}

void BattalionHQ::cancelRelocation()
{
    if (!isRelocating_) return;

    // Emergency halt — stay at current position, bring systems back online
    isRelocating_ = false;
    relocateTimeRemaining_ = 0.0f;

    // Need setup time to get operational again at current position
    status_ = HQStatus::SETTING_UP;
    relocateTimeRemaining_ = SETUP_TIME;
    isRelocating_ = true;  // Reuse relocation countdown for setup
    relocateDestination_ = position_;  // Stay in place
}

std::string BattalionHQ::getStatusString() const
{
    switch (status_) {
        case HQStatus::OPERATIONAL: return "OPERATIONAL";
        case HQStatus::RELOCATING:  return "RELOCATING";
        case HQStatus::SETTING_UP:  return "SETTING UP";
        case HQStatus::DEGRADED:    return "DEGRADED";
    }
    return "UNKNOWN";
}

HQData BattalionHQ::getData() const
{
    HQData data;
    data.designation = designation_;
    data.status = status_;
    data.position = position_;
    data.relocateDestination = relocateDestination_;
    data.isRelocating = isRelocating_;
    data.relocateTimeRemaining = relocateTimeRemaining_;
    data.totalRelocateTime = TOTAL_RELOCATE_TIME;
    data.personnelCount = HQ_PERSONNEL;
    data.radarOnline = radarOnline_;
    data.commsOnline = commsOnline_;
    data.statusString = getStatusString();
    return data;
}

std::string HQData::format() const
{
    char buf[512];
    snprintf(buf, sizeof(buf),
        "=== BATTALION HQ ===\n"
        "  UNIT:   %s\n"
        "  STATUS: %s\n"
        "  POS:    %.1f km / %03d deg\n"
        "  RADAR:  %s\n"
        "  COMMS:  %s\n"
        "  STAFF:  %d personnel\n",
        designation.c_str(),
        statusString.c_str(),
        position.range, (int)position.azimuth,
        radarOnline ? "ONLINE (AN/TPS-43E)" : "OFFLINE",
        commsOnline ? "ONLINE (encrypted)" : "OFFLINE",
        personnelCount);

    std::string result(buf);

    if (isRelocating) {
        char reloc[128];
        snprintf(reloc, sizeof(reloc),
            "  DEST:   %.1f km / %03d deg\n"
            "  ETA:    %.0fs remaining\n",
            relocateDestination.range, (int)relocateDestination.azimuth,
            relocateTimeRemaining);
        result += reloc;
    }

    return result;
}
