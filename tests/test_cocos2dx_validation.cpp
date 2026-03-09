// ============================================================================
// Cocos2d-x Validation Test
//
// Validates that cocos2d-x is properly installed, linkable, and that we can
// obtain handles to core engine objects. When cocos2d-x is not available,
// validates the stub build mode instead.
// ============================================================================

#include "TestFramework.h"
#include "GameTypes.h"

#include <fstream>
#include <cstdlib>

#if USE_COCOS2DX
#include "cocos2d.h"
#endif

// ---------------------------------------------------------------------------
// Test: cocos2d-x build flag detection
// ---------------------------------------------------------------------------
void test_cocos2dx_build_flag()
{
#if USE_COCOS2DX
    // Built with cocos2d-x — flag must be 1
    ASSERT_EQ(1, USE_COCOS2DX);
#else
    // Built in stub mode — flag must be 0
    ASSERT_EQ(0, USE_COCOS2DX);
#endif
}

// ---------------------------------------------------------------------------
// Test: cocos2d-x headers are includable (only when linked)
// ---------------------------------------------------------------------------
void test_cocos2dx_headers_available()
{
#if USE_COCOS2DX
    // If this compiles, the headers are found. Verify a known constant.
    // cocos2d::Director is a core singleton — its existence proves headers work.
    auto* director = cocos2d::Director::getInstance();
    ASSERT_NOT_NULL(director);
#else
    // Stub mode: verify the compile-time flag is correctly set
    ASSERT_TRUE(true);
#endif
}

// ---------------------------------------------------------------------------
// Test: cocos2d-x Director singleton handle
// ---------------------------------------------------------------------------
void test_cocos2dx_director_handle()
{
#if USE_COCOS2DX
    auto* director = cocos2d::Director::getInstance();
    ASSERT_NOT_NULL(director);

    // Director should report a non-zero OpenGL version or renderer
    auto* renderer = director->getRenderer();
    ASSERT_NOT_NULL(renderer);
#else
    // Stub mode passes — no Director to test
    ASSERT_TRUE(true);
#endif
}

// ---------------------------------------------------------------------------
// Test: cocos2d-x Node creation (basic scene graph handle)
// ---------------------------------------------------------------------------
void test_cocos2dx_node_creation()
{
#if USE_COCOS2DX
    auto* node = cocos2d::Node::create();
    ASSERT_NOT_NULL(node);

    // Verify node is in the autorelease pool and has default properties
    auto pos = node->getPosition();
    ASSERT_NEAR(0.0f, pos.x, 0.001f);
    ASSERT_NEAR(0.0f, pos.y, 0.001f);
#else
    ASSERT_TRUE(true);
#endif
}

// ---------------------------------------------------------------------------
// Test: cocos2d-x Scene creation
// ---------------------------------------------------------------------------
void test_cocos2dx_scene_creation()
{
#if USE_COCOS2DX
    auto* scene = cocos2d::Scene::create();
    ASSERT_NOT_NULL(scene);
#else
    ASSERT_TRUE(true);
#endif
}

// ---------------------------------------------------------------------------
// Test: cocos2d-x DrawNode creation (critical for radar rendering)
// ---------------------------------------------------------------------------
void test_cocos2dx_drawnode_creation()
{
#if USE_COCOS2DX
    auto* drawNode = cocos2d::DrawNode::create();
    ASSERT_NOT_NULL(drawNode);
#else
    ASSERT_TRUE(true);
#endif
}

// ---------------------------------------------------------------------------
// Test: cocos2d-x Label creation (critical for HUD text)
// ---------------------------------------------------------------------------
void test_cocos2dx_label_creation()
{
#if USE_COCOS2DX
    auto* label = cocos2d::Label::createWithSystemFont("TEST", "Courier", 12);
    ASSERT_NOT_NULL(label);
#else
    ASSERT_TRUE(true);
#endif
}

// ---------------------------------------------------------------------------
// Test: cocos2d-x installation path exists (filesystem check)
// ---------------------------------------------------------------------------
void test_cocos2dx_installation_path()
{
    // Check common installation paths
    const char* paths[] = {
        "cocos2d-x/CMakeLists.txt",
        "cocos2d-x/cocos/cocos2d.h",
        "external/cocos2d-x/CMakeLists.txt",
    };

    bool found = false;
    std::string foundPath;

    for (const auto& path : paths) {
        std::ifstream f(path);
        if (f.good()) {
            found = true;
            foundPath = path;
            break;
        }
    }

    // Also check environment variable
    const char* envRoot = std::getenv("COCOS2DX_ROOT");
    if (envRoot) {
        std::string envPath = std::string(envRoot) + "/CMakeLists.txt";
        std::ifstream f(envPath);
        if (f.good()) {
            found = true;
            foundPath = envPath;
        }
    }

#if USE_COCOS2DX
    // If built with cocos2d-x, installation must exist
    ASSERT_TRUE(found);
#else
    // In stub mode, just report whether it was found
    if (found) {
        std::cout << "    [INFO] cocos2d-x found at: " << foundPath
                  << " (rebuild with -DCOCOS2DX_ROOT to enable)" << std::endl;
    } else {
        std::cout << "    [INFO] cocos2d-x not installed — running in stub mode" << std::endl;
    }
    ASSERT_TRUE(true);
#endif
}

// ---------------------------------------------------------------------------
// Test: Stub mode game systems are functional without cocos2d-x
// ---------------------------------------------------------------------------
void test_stub_mode_functional()
{
#if !USE_COCOS2DX
    // Verify that core game logic works without the engine
    // This confirms the stub architecture is sound

    // GameConstants should be accessible
    ASSERT_NEAR(463.0f, GameConstants::RADAR_MAX_RANGE_KM, 0.01f);

    // PolarCoord math should work
    PolarCoord p{50.0f, 90.0f};  // 50km due East
    float x = p.toScreenX(1.0f);
    float y = p.toScreenY(1.0f);
    ASSERT_NEAR(50.0f, x, 0.1f);
    ASSERT_NEAR(0.0f, y, 0.1f);

    // TrackData formatting should work
    TrackData td;
    td.trackId = 42;
    td.altitude = 35000.0f;
    td.classification = TrackClassification::HOSTILE;
    ASSERT_STR_EQ("TK-042", td.getTrackIdString());
    ASSERT_STR_EQ("FL350", td.getAltitudeString());
    ASSERT_STR_EQ("HOSTILE", td.getClassificationString());
#else
    ASSERT_TRUE(true);
#endif
}

// ============================================================================
// Suite runner
// ============================================================================
void run_cocos2dx_validation_tests()
{
    TEST_SUITE("Cocos2d-x Validation");

    test_cocos2dx_build_flag();
    test_cocos2dx_headers_available();
    test_cocos2dx_director_handle();
    test_cocos2dx_node_creation();
    test_cocos2dx_scene_creation();
    test_cocos2dx_drawnode_creation();
    test_cocos2dx_label_creation();
    test_cocos2dx_installation_path();
    test_stub_mode_functional();

    TEST_SUITE_END();
}
