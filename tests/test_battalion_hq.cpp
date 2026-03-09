// ============================================================================
// Battalion HQ Unit Tests — Mobile AN/TSQ-73 Missile Minder
// ============================================================================

#include "TestFramework.h"
#include "BattalionHQ.h"

// ---------------------------------------------------------------------------
// Construction and init
// ---------------------------------------------------------------------------
void test_hq_construction()
{
    BattalionHQ hq;
    // Before init, not operational
    ASSERT_FALSE(hq.isRadarOnline());
}

void test_hq_init()
{
    BattalionHQ hq;
    hq.init(2.0f, 0.0f);
    ASSERT_TRUE(hq.isOperational());
    ASSERT_TRUE(hq.isRadarOnline());
    ASSERT_TRUE(hq.isCommsOnline());
}

void test_hq_init_position()
{
    BattalionHQ hq;
    hq.init(2.0f, 45.0f);
    auto pos = hq.getPosition();
    ASSERT_NEAR(2.0f, pos.range, 0.01f);
    ASSERT_NEAR(45.0f, pos.azimuth, 0.01f);
}

// ---------------------------------------------------------------------------
// Relocation
// ---------------------------------------------------------------------------
void test_hq_relocate_starts()
{
    BattalionHQ hq;
    hq.init(2.0f, 0.0f);
    ASSERT_TRUE(hq.relocate(5.0f, 180.0f));
    ASSERT_TRUE(hq.isRelocating());
}

void test_hq_relocate_goes_offline()
{
    BattalionHQ hq;
    hq.init(2.0f, 0.0f);
    hq.relocate(5.0f, 180.0f);
    ASSERT_FALSE(hq.isRadarOnline());
    ASSERT_FALSE(hq.isCommsOnline());
}

void test_hq_relocate_status()
{
    BattalionHQ hq;
    hq.init(2.0f, 0.0f);
    hq.relocate(5.0f, 180.0f);
    ASSERT_TRUE(hq.getStatus() == HQStatus::RELOCATING);
}

void test_hq_cannot_relocate_twice()
{
    BattalionHQ hq;
    hq.init(2.0f, 0.0f);
    hq.relocate(5.0f, 180.0f);
    ASSERT_FALSE(hq.relocate(10.0f, 270.0f));
}

void test_hq_relocate_completes()
{
    BattalionHQ hq;
    hq.init(2.0f, 0.0f);
    hq.relocate(5.0f, 180.0f);

    // Total relocation: 90 seconds
    for (int t = 0; t < 95; t++) hq.update(1.0f);

    ASSERT_TRUE(hq.isOperational());
    ASSERT_TRUE(hq.isRadarOnline());
    ASSERT_TRUE(hq.isCommsOnline());
}

void test_hq_relocate_updates_position()
{
    BattalionHQ hq;
    hq.init(2.0f, 0.0f);
    hq.relocate(5.0f, 180.0f);

    for (int t = 0; t < 95; t++) hq.update(1.0f);

    auto pos = hq.getPosition();
    ASSERT_NEAR(5.0f, pos.range, 0.01f);
    ASSERT_NEAR(180.0f, pos.azimuth, 0.01f);
}

void test_hq_relocate_time()
{
    ASSERT_NEAR(90.0f, BattalionHQ::TOTAL_RELOCATE_TIME, 0.01f);
}

// ---------------------------------------------------------------------------
// Relocation phases
// ---------------------------------------------------------------------------
void test_hq_setting_up_phase()
{
    BattalionHQ hq;
    hq.init(2.0f, 0.0f);
    hq.relocate(5.0f, 180.0f);

    // After 65 seconds (past teardown+move, in setup phase)
    for (int t = 0; t < 65; t++) hq.update(1.0f);

    ASSERT_TRUE(hq.getStatus() == HQStatus::SETTING_UP);
}

// ---------------------------------------------------------------------------
// Data and formatting
// ---------------------------------------------------------------------------
void test_hq_data()
{
    BattalionHQ hq;
    hq.init(2.0f, 0.0f);

    auto data = hq.getData();
    ASSERT_STR_EQ("MINDER-HQ", data.designation);
    ASSERT_TRUE(data.radarOnline);
    ASSERT_EQ(BattalionHQ::HQ_PERSONNEL, data.personnelCount);
}

void test_hq_data_format()
{
    BattalionHQ hq;
    hq.init(2.0f, 0.0f);

    auto data = hq.getData();
    auto text = data.format();
    ASSERT_GT((int)text.size(), 0);
}

void test_hq_status_string()
{
    BattalionHQ hq;
    hq.init(2.0f, 0.0f);
    ASSERT_STR_EQ("OPERATIONAL", hq.getStatusString());
}

void test_hq_relocating_status_string()
{
    BattalionHQ hq;
    hq.init(2.0f, 0.0f);
    hq.relocate(5.0f, 180.0f);
    ASSERT_STR_EQ("RELOCATING", hq.getStatusString());
}

// ---------------------------------------------------------------------------
// Personnel
// ---------------------------------------------------------------------------
void test_hq_personnel_count()
{
    ASSERT_EQ(35, BattalionHQ::HQ_PERSONNEL);
}

// ============================================================================
void run_battalion_hq_tests()
{
    TEST_SUITE("Battalion HQ");

    test_hq_construction();
    test_hq_init();
    test_hq_init_position();
    test_hq_relocate_starts();
    test_hq_relocate_goes_offline();
    test_hq_relocate_status();
    test_hq_cannot_relocate_twice();
    test_hq_relocate_completes();
    test_hq_relocate_updates_position();
    test_hq_relocate_time();
    test_hq_setting_up_phase();
    test_hq_data();
    test_hq_data_format();
    test_hq_status_string();
    test_hq_relocating_status_string();
    test_hq_personnel_count();

    TEST_SUITE_END();
}
