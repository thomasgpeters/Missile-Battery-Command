// ============================================================================
// Missile Battery Command — Test Suite Entry Point
//
// Runs all test suites and reports results.
// Exit code: 0 = all pass, 1 = failures detected.
// ============================================================================

#include "TestFramework.h"
#include <iostream>

// Forward declarations for all test suites
void run_cocos2dx_validation_tests();
void run_game_types_tests();
void run_aircraft_tests();
void run_iff_system_tests();
void run_track_manager_tests();
void run_missile_battery_tests();
void run_aircraft_generator_tests();
void run_fire_control_tests();
void run_game_config_tests();
void run_airspace_manager_tests();
void run_data_card_tests();
void run_threat_board_tests();
void run_battalion_hq_tests();

int main(int argc, char** argv)
{
    std::cout << "========================================" << std::endl;
    std::cout << "  MISSILE BATTERY COMMAND" << std::endl;
    std::cout << "  Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

#if USE_COCOS2DX
    std::cout << "  Build mode: COCOS2D-X (full)" << std::endl;
#else
    std::cout << "  Build mode: STUB (no cocos2d-x)" << std::endl;
#endif

    std::cout << std::endl;

    // Run all test suites
    run_cocos2dx_validation_tests();
    run_game_types_tests();
    run_aircraft_tests();
    run_iff_system_tests();
    run_track_manager_tests();
    run_missile_battery_tests();
    run_aircraft_generator_tests();
    run_fire_control_tests();
    run_game_config_tests();
    run_airspace_manager_tests();
    run_data_card_tests();
    run_threat_board_tests();
    run_battalion_hq_tests();

    // Print summary and return exit code
    return TEST_SUMMARY();
}
