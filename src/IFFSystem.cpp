#include "IFFSystem.h"
#include <random>
#include <algorithm>

IFFSystem::IFFSystem()
    : errorRate_(0.0f)
{
}

IFFSystem::~IFFSystem() {}

void IFFSystem::interrogate(Aircraft* aircraft)
{
    if (!aircraft || aircraft->getTrackId() < 0) return;

    // Don't interrogate if already being interrogated
    if (isInterrogating(aircraft->getTrackId())) return;

    // Don't re-interrogate if already has a result
    if (aircraft->getIFFStatus() != IFFStatus::PENDING) return;

    IFFInterrogation interrog;
    interrog.trackId = aircraft->getTrackId();
    interrog.timeRemaining = GameConstants::IFF_INTERROGATION_TIME;
    interrog.aircraft = aircraft;

    pendingInterrogations_.push_back(interrog);
}

void IFFSystem::update(float dt)
{
    auto it = pendingInterrogations_.begin();
    while (it != pendingInterrogations_.end()) {
        it->timeRemaining -= dt;

        if (it->timeRemaining <= 0.0f) {
            // Interrogation complete - determine result
            if (it->aircraft && it->aircraft->isAlive()) {
                IFFStatus result = determineResult(it->aircraft);
                it->aircraft->setIFFStatus(result);
            }
            it = pendingInterrogations_.erase(it);
        } else {
            ++it;
        }
    }
}

bool IFFSystem::isInterrogating(int trackId) const
{
    for (const auto& interrog : pendingInterrogations_) {
        if (interrog.trackId == trackId) return true;
    }
    return false;
}

IFFStatus IFFSystem::determineResult(Aircraft* aircraft)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    bool actuallyFriendly = aircraft->isFriendly();

    // Check for IFF error
    if (dist(rng) < errorRate_) {
        // Error! Return incorrect or inconclusive result
        float errorType = dist(rng);
        if (errorType < 0.3f) {
            // Complete misidentification (dangerous!)
            return actuallyFriendly ? IFFStatus::HOSTILE : IFFStatus::FRIENDLY;
        } else {
            // Inconclusive
            return IFFStatus::UNKNOWN;
        }
    }

    // Stealth aircraft have a chance of appearing as UNKNOWN
    if (aircraft->getType() == AircraftType::STEALTH_FIGHTER) {
        if (dist(rng) < 0.3f) {
            return IFFStatus::UNKNOWN;
        }
    }

    // Normal operation - correct identification
    return actuallyFriendly ? IFFStatus::FRIENDLY : IFFStatus::HOSTILE;
}
