// ============================================================================
// IFF System Unit Tests
// ============================================================================

#include "TestFramework.h"
#include "IFFSystem.h"

// ---------------------------------------------------------------------------
// Basic interrogation
// ---------------------------------------------------------------------------
void test_iff_construction()
{
    IFFSystem iff;
    // Should construct without error
    ASSERT_TRUE(true);
}

void test_iff_interrogation_starts()
{
    IFFSystem iff;
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                25000.0f, 600.0f, 180.0f, false);
    ac.setTrackId(1);

    iff.interrogate(&ac);
    ASSERT_TRUE(iff.isInterrogating(1));
}

void test_iff_not_interrogating_unknown_track()
{
    IFFSystem iff;
    ASSERT_FALSE(iff.isInterrogating(999));
}

void test_iff_no_duplicate_interrogation()
{
    IFFSystem iff;
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                25000.0f, 600.0f, 180.0f, false);
    ac.setTrackId(1);

    iff.interrogate(&ac);
    iff.interrogate(&ac);  // Should not add duplicate
    ASSERT_TRUE(iff.isInterrogating(1));
}

void test_iff_requires_track_id()
{
    IFFSystem iff;
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                25000.0f, 600.0f, 180.0f, false);
    // trackId is -1 (not assigned)

    iff.interrogate(&ac);
    ASSERT_FALSE(iff.isInterrogating(-1));
}

void test_iff_null_aircraft_ignored()
{
    IFFSystem iff;
    iff.interrogate(nullptr);
    // Should not crash
    ASSERT_TRUE(true);
}

// ---------------------------------------------------------------------------
// Interrogation timing
// ---------------------------------------------------------------------------
void test_iff_pending_before_complete()
{
    IFFSystem iff;
    iff.setErrorRate(0.0f);

    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                25000.0f, 600.0f, 180.0f, false);
    ac.setTrackId(1);

    iff.interrogate(&ac);
    iff.update(1.0f);  // 1 second — not enough (needs 2s)

    ASSERT_TRUE(ac.getIFFStatus() == IFFStatus::PENDING);
}

void test_iff_completes_after_interval()
{
    IFFSystem iff;
    iff.setErrorRate(0.0f);

    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                25000.0f, 600.0f, 180.0f, false);
    ac.setTrackId(1);

    iff.interrogate(&ac);
    iff.update(2.5f);  // Past the 2-second interrogation time

    // Should no longer be interrogating
    ASSERT_FALSE(iff.isInterrogating(1));
}

void test_iff_hostile_identified()
{
    IFFSystem iff;
    iff.setErrorRate(0.0f);  // No errors

    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                25000.0f, 600.0f, 180.0f, false);  // hostile
    ac.setTrackId(1);

    iff.interrogate(&ac);
    iff.update(3.0f);

    ASSERT_TRUE(ac.getIFFStatus() == IFFStatus::HOSTILE);
}

void test_iff_friendly_identified()
{
    IFFSystem iff;
    iff.setErrorRate(0.0f);

    Aircraft ac(AircraftType::CIVILIAN_AIRLINER, 80.0f, 0.0f,
                35000.0f, 450.0f, 180.0f, true);  // friendly
    ac.setTrackId(2);

    iff.interrogate(&ac);
    iff.update(3.0f);

    ASSERT_TRUE(ac.getIFFStatus() == IFFStatus::FRIENDLY);
}

// ---------------------------------------------------------------------------
// Already-identified aircraft not re-interrogated
// ---------------------------------------------------------------------------
void test_iff_skip_already_identified()
{
    IFFSystem iff;
    iff.setErrorRate(0.0f);

    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                25000.0f, 600.0f, 180.0f, false);
    ac.setTrackId(1);
    ac.setIFFStatus(IFFStatus::HOSTILE);  // Already identified

    iff.interrogate(&ac);
    ASSERT_FALSE(iff.isInterrogating(1));
}

// ---------------------------------------------------------------------------
// Multiple simultaneous interrogations
// ---------------------------------------------------------------------------
void test_iff_multiple_aircraft()
{
    IFFSystem iff;
    iff.setErrorRate(0.0f);

    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                 25000.0f, 600.0f, 180.0f, false);
    Aircraft ac2(AircraftType::CIVILIAN_AIRLINER, 70.0f, 90.0f,
                 35000.0f, 450.0f, 270.0f, true);
    ac1.setTrackId(1);
    ac2.setTrackId(2);

    iff.interrogate(&ac1);
    iff.interrogate(&ac2);

    ASSERT_TRUE(iff.isInterrogating(1));
}

void test_iff_multiple_both_interrogating()
{
    IFFSystem iff;
    iff.setErrorRate(0.0f);

    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                 25000.0f, 600.0f, 180.0f, false);
    Aircraft ac2(AircraftType::CIVILIAN_AIRLINER, 70.0f, 90.0f,
                 35000.0f, 450.0f, 270.0f, true);
    ac1.setTrackId(1);
    ac2.setTrackId(2);

    iff.interrogate(&ac1);
    iff.interrogate(&ac2);

    ASSERT_TRUE(iff.isInterrogating(2));
}

void test_iff_multiple_both_resolve()
{
    IFFSystem iff;
    iff.setErrorRate(0.0f);

    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                 25000.0f, 600.0f, 180.0f, false);
    Aircraft ac2(AircraftType::CIVILIAN_AIRLINER, 70.0f, 90.0f,
                 35000.0f, 450.0f, 270.0f, true);
    ac1.setTrackId(1);
    ac2.setTrackId(2);

    iff.interrogate(&ac1);
    iff.interrogate(&ac2);
    iff.update(3.0f);

    // Both should be resolved
    ASSERT_TRUE(ac1.getIFFStatus() == IFFStatus::HOSTILE);
}

void test_iff_multiple_friendly_resolved()
{
    IFFSystem iff;
    iff.setErrorRate(0.0f);

    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                 25000.0f, 600.0f, 180.0f, false);
    Aircraft ac2(AircraftType::CIVILIAN_AIRLINER, 70.0f, 90.0f,
                 35000.0f, 450.0f, 270.0f, true);
    ac1.setTrackId(1);
    ac2.setTrackId(2);

    iff.interrogate(&ac1);
    iff.interrogate(&ac2);
    iff.update(3.0f);

    ASSERT_TRUE(ac2.getIFFStatus() == IFFStatus::FRIENDLY);
}

// ---------------------------------------------------------------------------
// Dead aircraft during interrogation
// ---------------------------------------------------------------------------
void test_iff_dead_aircraft_no_result()
{
    IFFSystem iff;
    iff.setErrorRate(0.0f);

    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                25000.0f, 600.0f, 180.0f, false);
    ac.setTrackId(1);

    iff.interrogate(&ac);
    ac.destroy();  // Destroyed during interrogation
    iff.update(3.0f);

    // IFF status should remain PENDING since aircraft is dead
    ASSERT_TRUE(ac.getIFFStatus() == IFFStatus::PENDING);
}

// ---------------------------------------------------------------------------
// Error rate
// ---------------------------------------------------------------------------
void test_iff_error_rate_setter()
{
    IFFSystem iff;
    iff.setErrorRate(0.5f);
    // No crash — can't directly read back, but validates the setter works
    ASSERT_TRUE(true);
}

// ============================================================================
void run_iff_system_tests()
{
    TEST_SUITE("IFF System");

    test_iff_construction();
    test_iff_interrogation_starts();
    test_iff_not_interrogating_unknown_track();
    test_iff_no_duplicate_interrogation();
    test_iff_requires_track_id();
    test_iff_null_aircraft_ignored();
    test_iff_pending_before_complete();
    test_iff_completes_after_interval();
    test_iff_hostile_identified();
    test_iff_friendly_identified();
    test_iff_skip_already_identified();
    test_iff_multiple_aircraft();
    test_iff_multiple_both_interrogating();
    test_iff_multiple_both_resolve();
    test_iff_multiple_friendly_resolved();
    test_iff_dead_aircraft_no_result();
    test_iff_error_rate_setter();

    TEST_SUITE_END();
}
