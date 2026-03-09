// ============================================================================
// Game Config Unit Tests
// ============================================================================

#include "TestFramework.h"
#include "GameConfig.h"

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------
void test_config_singleton()
{
    auto& cfg1 = GameConfig::getInstance();
    auto& cfg2 = GameConfig::getInstance();
    ASSERT_TRUE(&cfg1 == &cfg2);
}

// ---------------------------------------------------------------------------
// Level setting
// ---------------------------------------------------------------------------
void test_config_default_level()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(1);
    ASSERT_EQ(1, cfg.getLevel());
}

void test_config_set_level()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(3);
    ASSERT_EQ(3, cfg.getLevel());
}

void test_config_clamp_level_low()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(0);
    ASSERT_EQ(1, cfg.getLevel());
}

void test_config_clamp_level_high()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(99);
    ASSERT_EQ(5, cfg.getLevel());
}

// ---------------------------------------------------------------------------
// Level 1 configuration
// ---------------------------------------------------------------------------
void test_config_level1_max_aircraft()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(1);
    ASSERT_EQ(3, cfg.getMaxConcurrentAircraft());
}

void test_config_level1_no_stealth()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(1);
    ASSERT_FALSE(cfg.isStealthEnabled());
}

void test_config_level1_no_iff_errors()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(1);
    ASSERT_NEAR(0.0f, cfg.getIFFErrorRate(), 0.001f);
}

void test_config_level1_speed_slow()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(1);
    ASSERT_LT(cfg.getSpeedMultiplier(), 1.0f);
}

void test_config_level1_friendly_ratio()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(1);
    ASSERT_NEAR(0.30f, cfg.getFriendlyRatio(), 0.01f);
}

// ---------------------------------------------------------------------------
// Level 5 configuration
// ---------------------------------------------------------------------------
void test_config_level5_max_aircraft()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(5);
    ASSERT_EQ(15, cfg.getMaxConcurrentAircraft());
}

void test_config_level5_stealth_enabled()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(5);
    ASSERT_TRUE(cfg.isStealthEnabled());
}

void test_config_level5_iff_errors()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(5);
    ASSERT_GT(cfg.getIFFErrorRate(), 0.0f);
}

void test_config_level5_speed_fast()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(5);
    ASSERT_GT(cfg.getSpeedMultiplier(), 1.0f);
}

void test_config_level5_fewer_friendlies()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(5);
    ASSERT_LT(cfg.getFriendlyRatio(), 0.15f);
}

// ---------------------------------------------------------------------------
// Difficulty scales monotonically
// ---------------------------------------------------------------------------
void test_config_max_aircraft_increases()
{
    auto& cfg = GameConfig::getInstance();

    cfg.setLevel(1);
    int level1 = cfg.getMaxConcurrentAircraft();
    cfg.setLevel(5);
    int level5 = cfg.getMaxConcurrentAircraft();

    ASSERT_GT(level5, level1);
}

void test_config_speed_increases()
{
    auto& cfg = GameConfig::getInstance();

    cfg.setLevel(1);
    float speed1 = cfg.getSpeedMultiplier();
    cfg.setLevel(5);
    float speed5 = cfg.getSpeedMultiplier();

    ASSERT_GT(speed5, speed1);
}

void test_config_friendly_ratio_decreases()
{
    auto& cfg = GameConfig::getInstance();

    cfg.setLevel(1);
    float ratio1 = cfg.getFriendlyRatio();
    cfg.setLevel(5);
    float ratio5 = cfg.getFriendlyRatio();

    ASSERT_LT(ratio5, ratio1);
}

// ---------------------------------------------------------------------------
// Spawn interval
// ---------------------------------------------------------------------------
void test_config_spawn_interval_positive()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(1);
    float interval = cfg.getSpawnInterval();
    ASSERT_GT(interval, 0.0f);
}

// ---------------------------------------------------------------------------
// Scoring
// ---------------------------------------------------------------------------
void test_config_score_strategic_bomber()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(1);
    int score = cfg.getHostileDestroyedScore(AircraftType::STRATEGIC_BOMBER);
    ASSERT_EQ(500, score);
}

void test_config_score_fighter()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(1);
    int score = cfg.getHostileDestroyedScore(AircraftType::FIGHTER_ATTACK);
    ASSERT_EQ(300, score);
}

void test_config_score_recon_drone()
{
    auto& cfg = GameConfig::getInstance();
    cfg.setLevel(1);
    int score = cfg.getHostileDestroyedScore(AircraftType::RECON_DRONE);
    ASSERT_EQ(100, score);
}

void test_config_score_higher_at_higher_level()
{
    auto& cfg = GameConfig::getInstance();

    cfg.setLevel(1);
    int score1 = cfg.getHostileDestroyedScore(AircraftType::FIGHTER_ATTACK);
    cfg.setLevel(5);
    int score5 = cfg.getHostileDestroyedScore(AircraftType::FIGHTER_ATTACK);

    ASSERT_GT(score5, score1);
}

// ============================================================================
void run_game_config_tests()
{
    TEST_SUITE("Game Config");

    test_config_singleton();
    test_config_default_level();
    test_config_set_level();
    test_config_clamp_level_low();
    test_config_clamp_level_high();
    test_config_level1_max_aircraft();
    test_config_level1_no_stealth();
    test_config_level1_no_iff_errors();
    test_config_level1_speed_slow();
    test_config_level1_friendly_ratio();
    test_config_level5_max_aircraft();
    test_config_level5_stealth_enabled();
    test_config_level5_iff_errors();
    test_config_level5_speed_fast();
    test_config_level5_fewer_friendlies();
    test_config_max_aircraft_increases();
    test_config_speed_increases();
    test_config_friendly_ratio_decreases();
    test_config_spawn_interval_positive();
    test_config_score_strategic_bomber();
    test_config_score_fighter();
    test_config_score_recon_drone();
    test_config_score_higher_at_higher_level();

    // Reset to level 1 for other tests
    GameConfig::getInstance().setLevel(1);

    TEST_SUITE_END();
}
