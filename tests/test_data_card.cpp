// ============================================================================
// Data Card Unit Tests
// ============================================================================

#include "TestFramework.h"
#include "DataCard.h"

// ---------------------------------------------------------------------------
// Equipment cards
// ---------------------------------------------------------------------------
void test_patriot_equipment_card()
{
    auto card = DataCardManager::getEquipmentCard(BatteryType::PATRIOT);
    ASSERT_NEAR(GameConstants::PATRIOT_MAX_RANGE, card.maxRangeKm, 0.01f);
    ASSERT_EQ(GameConstants::PATRIOT_MAX_MISSILES, card.launcherCapacity);
}

void test_patriot_card_radar()
{
    auto card = DataCardManager::getEquipmentCard(BatteryType::PATRIOT);
    ASSERT_STR_EQ("AN/MPQ-53 Phased Array", card.radarSystem);
}

void test_patriot_card_guidance()
{
    auto card = DataCardManager::getEquipmentCard(BatteryType::PATRIOT);
    ASSERT_STR_EQ("Track-Via-Missile (TVM)", card.guidanceType);
}

void test_hawk_equipment_card()
{
    auto card = DataCardManager::getEquipmentCard(BatteryType::HAWK);
    ASSERT_NEAR(GameConstants::HAWK_MAX_RANGE, card.maxRangeKm, 0.01f);
    ASSERT_EQ(GameConstants::HAWK_MAX_MISSILES, card.launcherCapacity);
}

void test_hawk_card_guidance()
{
    auto card = DataCardManager::getEquipmentCard(BatteryType::HAWK);
    ASSERT_STR_EQ("Semi-Active Radar Homing (SARH)", card.guidanceType);
}

void test_javelin_equipment_card()
{
    auto card = DataCardManager::getEquipmentCard(BatteryType::JAVELIN);
    ASSERT_NEAR(GameConstants::JAVELIN_MAX_RANGE, card.maxRangeKm, 0.01f);
    ASSERT_EQ(GameConstants::JAVELIN_MAX_MISSILES, card.launcherCapacity);
}

void test_javelin_card_guidance()
{
    auto card = DataCardManager::getEquipmentCard(BatteryType::JAVELIN);
    ASSERT_STR_EQ("Imaging IR (fire-and-forget)", card.guidanceType);
}

void test_all_equipment_cards()
{
    auto cards = DataCardManager::getAllEquipmentCards();
    ASSERT_EQ(3, (int)cards.size());
}

// ---------------------------------------------------------------------------
// Troop strength cards
// ---------------------------------------------------------------------------
void test_patriot_troop_strength()
{
    auto card = DataCardManager::getTroopStrengthCard(BatteryType::PATRIOT, "PATRIOT-1");
    ASSERT_STR_EQ("PATRIOT-1", card.unitDesignation);
    ASSERT_GT(card.personnelCount, 0);
    ASSERT_EQ(card.personnelCount, card.officerCount + card.enlistedCount);
}

void test_hawk_troop_strength()
{
    auto card = DataCardManager::getTroopStrengthCard(BatteryType::HAWK, "HAWK-2");
    ASSERT_STR_EQ("HAWK-2", card.unitDesignation);
    ASSERT_EQ(GameConstants::HAWK_LOADERS, card.loaderCount);
}

void test_javelin_troop_strength()
{
    auto card = DataCardManager::getTroopStrengthCard(BatteryType::JAVELIN, "JAVELIN-1");
    ASSERT_STR_EQ("JAVELIN-1", card.unitDesignation);
    ASSERT_GT(card.operatorCount, 0);
}

// ---------------------------------------------------------------------------
// Card formatting (no crash)
// ---------------------------------------------------------------------------
void test_equipment_card_format()
{
    auto card = DataCardManager::getEquipmentCard(BatteryType::PATRIOT);
    auto str = card.format();
    ASSERT_GT((int)str.size(), 0);
}

void test_troop_card_format()
{
    auto card = DataCardManager::getTroopStrengthCard(BatteryType::HAWK, "HAWK-1");
    auto str = card.format();
    ASSERT_GT((int)str.size(), 0);
}

void test_intel_card_format()
{
    BatteryIntelCard card;
    card.designation = "PATRIOT-1";
    card.currentPosition = {15.0f, 0.0f};
    card.isRelocating = false;
    card.relocateTimeRemaining = 0.0f;
    card.missilesReady = 4;
    card.missilesInStock = 4;
    card.currentStatus = "READY";
    card.engagementsTotal = 5;
    card.hits = 3;
    card.misses = 2;
    card.lastEngagement = "SPLASH TK-003";
    auto str = card.format();
    ASSERT_GT((int)str.size(), 0);
}

// ============================================================================
void run_data_card_tests()
{
    TEST_SUITE("Data Cards");

    test_patriot_equipment_card();
    test_patriot_card_radar();
    test_patriot_card_guidance();
    test_hawk_equipment_card();
    test_hawk_card_guidance();
    test_javelin_equipment_card();
    test_javelin_card_guidance();
    test_all_equipment_cards();
    test_patriot_troop_strength();
    test_hawk_troop_strength();
    test_javelin_troop_strength();
    test_equipment_card_format();
    test_troop_card_format();
    test_intel_card_format();

    TEST_SUITE_END();
}
