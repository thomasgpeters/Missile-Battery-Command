#ifndef __TEST_FRAMEWORK_H__
#define __TEST_FRAMEWORK_H__

// ============================================================================
// Minimal test framework for Missile Battery Command
// No external dependencies — works anywhere the game compiles.
// ============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <cmath>
#include <cstring>

namespace mbc_test {

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

struct TestSuite {
    std::string name;
    std::vector<TestResult> results;
    int passed = 0;
    int failed = 0;
};

class TestRunner {
public:
    static TestRunner& instance() {
        static TestRunner runner;
        return runner;
    }

    void beginSuite(const std::string& name) {
        currentSuite_ = {name, {}, 0, 0};
        if (!quiet_) std::cout << "\n=== " << name << " ===" << std::endl;
    }

    void endSuite() {
        if (!quiet_) {
            std::cout << "--- " << currentSuite_.name << ": "
                      << currentSuite_.passed << " passed, "
                      << currentSuite_.failed << " failed ---\n";
        }
        suites_.push_back(currentSuite_);
        totalPassed_ += currentSuite_.passed;
        totalFailed_ += currentSuite_.failed;
    }

    void recordPass(const std::string& testName) {
        currentSuite_.results.push_back({testName, true, ""});
        currentSuite_.passed++;
        if (!quiet_) std::cout << "  PASS  " << testName << std::endl;
    }

    void recordFail(const std::string& testName, const std::string& msg) {
        currentSuite_.results.push_back({testName, false, msg});
        currentSuite_.failed++;
        if (!quiet_) std::cout << "  FAIL  " << testName << ": " << msg << std::endl;
    }

    int printSummary() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "  TEST SUMMARY" << std::endl;
        std::cout << "========================================" << std::endl;

        for (const auto& suite : suites_) {
            const char* status = (suite.failed == 0) ? "PASS" : "FAIL";
            std::cout << "  [" << status << "] " << suite.name
                      << " (" << suite.passed << "/"
                      << (suite.passed + suite.failed) << ")" << std::endl;
        }

        std::cout << "----------------------------------------" << std::endl;
        std::cout << "  Total: " << totalPassed_ << " passed, "
                  << totalFailed_ << " failed" << std::endl;
        std::cout << "========================================\n" << std::endl;

        return totalFailed_ > 0 ? 1 : 0;
    }

    void reset() {
        currentSuite_ = {};
        suites_.clear();
        totalPassed_ = 0;
        totalFailed_ = 0;
    }

    const std::vector<TestSuite>& getSuites() const { return suites_; }
    const TestSuite& getCurrentSuite() const { return currentSuite_; }
    int getTotalPassed() const { return totalPassed_; }
    int getTotalFailed() const { return totalFailed_; }

    void setQuiet(bool q) { quiet_ = q; }
    bool isQuiet() const { return quiet_; }

private:
    TestRunner() = default;
    TestSuite currentSuite_;
    std::vector<TestSuite> suites_;
    int totalPassed_ = 0;
    int totalFailed_ = 0;
    bool quiet_ = false;
};

// ============================================================================
// Test Registry — for interactive test harness
// ============================================================================

struct RegisteredTest {
    std::string name;
    std::string suite;
    std::function<void()> func;
    bool enabled = true;
};

class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry reg;
        return reg;
    }

    void registerTest(const std::string& suite, const std::string& name,
                      std::function<void()> func) {
        tests_.push_back({name, suite, func, true});
    }

    std::vector<RegisteredTest>& getTests() { return tests_; }
    const std::vector<RegisteredTest>& getTests() const { return tests_; }

    std::vector<std::string> getSuiteNames() const {
        std::vector<std::string> names;
        for (const auto& t : tests_) {
            bool found = false;
            for (const auto& n : names) {
                if (n == t.suite) { found = true; break; }
            }
            if (!found) names.push_back(t.suite);
        }
        return names;
    }

    std::vector<RegisteredTest*> getTestsForSuite(const std::string& suite) {
        std::vector<RegisteredTest*> result;
        for (auto& t : tests_) {
            if (t.suite == suite) result.push_back(&t);
        }
        return result;
    }

private:
    TestRegistry() = default;
    std::vector<RegisteredTest> tests_;
};

} // namespace mbc_test

// ============================================================================
// Test macros
// ============================================================================

#define TEST_SUITE(name) \
    mbc_test::TestRunner::instance().beginSuite(name)

#define TEST_SUITE_END() \
    mbc_test::TestRunner::instance().endSuite()

#define TEST_SUMMARY() \
    mbc_test::TestRunner::instance().printSummary()

#define ASSERT_TRUE(expr) \
    do { \
        if (expr) { \
            mbc_test::TestRunner::instance().recordPass(__func__); \
        } else { \
            std::ostringstream oss_; \
            oss_ << "Expected true: " << #expr << " [" << __FILE__ << ":" << __LINE__ << "]"; \
            mbc_test::TestRunner::instance().recordFail(__func__, oss_.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_FALSE(expr) \
    do { \
        if (!(expr)) { \
            mbc_test::TestRunner::instance().recordPass(__func__); \
        } else { \
            std::ostringstream oss_; \
            oss_ << "Expected false: " << #expr << " [" << __FILE__ << ":" << __LINE__ << "]"; \
            mbc_test::TestRunner::instance().recordFail(__func__, oss_.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_EQ(expected, actual) \
    do { \
        auto e_ = (expected); auto a_ = (actual); \
        if (e_ == a_) { \
            mbc_test::TestRunner::instance().recordPass(__func__); \
        } else { \
            std::ostringstream oss_; \
            oss_ << #expected << " == " << #actual << " (expected " << e_ << ", got " << a_ << ")" \
                 << " [" << __FILE__ << ":" << __LINE__ << "]"; \
            mbc_test::TestRunner::instance().recordFail(__func__, oss_.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_NE(expected, actual) \
    do { \
        auto e_ = (expected); auto a_ = (actual); \
        if (e_ != a_) { \
            mbc_test::TestRunner::instance().recordPass(__func__); \
        } else { \
            std::ostringstream oss_; \
            oss_ << #expected << " != " << #actual << " (both are " << a_ << ")" \
                 << " [" << __FILE__ << ":" << __LINE__ << "]"; \
            mbc_test::TestRunner::instance().recordFail(__func__, oss_.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_GT(a, b) \
    do { \
        auto a_ = (a); auto b_ = (b); \
        if (a_ > b_) { \
            mbc_test::TestRunner::instance().recordPass(__func__); \
        } else { \
            std::ostringstream oss_; \
            oss_ << #a << " > " << #b << " (" << a_ << " <= " << b_ << ")" \
                 << " [" << __FILE__ << ":" << __LINE__ << "]"; \
            mbc_test::TestRunner::instance().recordFail(__func__, oss_.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_GE(a, b) \
    do { \
        auto a_ = (a); auto b_ = (b); \
        if (a_ >= b_) { \
            mbc_test::TestRunner::instance().recordPass(__func__); \
        } else { \
            std::ostringstream oss_; \
            oss_ << #a << " >= " << #b << " (" << a_ << " < " << b_ << ")" \
                 << " [" << __FILE__ << ":" << __LINE__ << "]"; \
            mbc_test::TestRunner::instance().recordFail(__func__, oss_.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_LT(a, b) \
    do { \
        auto a_ = (a); auto b_ = (b); \
        if (a_ < b_) { \
            mbc_test::TestRunner::instance().recordPass(__func__); \
        } else { \
            std::ostringstream oss_; \
            oss_ << #a << " < " << #b << " (" << a_ << " >= " << b_ << ")" \
                 << " [" << __FILE__ << ":" << __LINE__ << "]"; \
            mbc_test::TestRunner::instance().recordFail(__func__, oss_.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_LE(a, b) \
    do { \
        auto a_ = (a); auto b_ = (b); \
        if (a_ <= b_) { \
            mbc_test::TestRunner::instance().recordPass(__func__); \
        } else { \
            std::ostringstream oss_; \
            oss_ << #a << " <= " << #b << " (" << a_ << " > " << b_ << ")" \
                 << " [" << __FILE__ << ":" << __LINE__ << "]"; \
            mbc_test::TestRunner::instance().recordFail(__func__, oss_.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_NEAR(expected, actual, tolerance) \
    do { \
        auto e_ = (expected); auto a_ = (actual); auto t_ = (tolerance); \
        if (std::fabs(e_ - a_) <= t_) { \
            mbc_test::TestRunner::instance().recordPass(__func__); \
        } else { \
            std::ostringstream oss_; \
            oss_ << #actual << " near " << #expected << " +/- " << t_ \
                 << " (expected " << e_ << ", got " << a_ << ")" \
                 << " [" << __FILE__ << ":" << __LINE__ << "]"; \
            mbc_test::TestRunner::instance().recordFail(__func__, oss_.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_STR_EQ(expected, actual) \
    do { \
        std::string e_(expected); std::string a_(actual); \
        if (e_ == a_) { \
            mbc_test::TestRunner::instance().recordPass(__func__); \
        } else { \
            std::ostringstream oss_; \
            oss_ << "Expected \"" << e_ << "\", got \"" << a_ << "\"" \
                 << " [" << __FILE__ << ":" << __LINE__ << "]"; \
            mbc_test::TestRunner::instance().recordFail(__func__, oss_.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) != nullptr) { \
            mbc_test::TestRunner::instance().recordPass(__func__); \
        } else { \
            std::ostringstream oss_; \
            oss_ << #ptr << " is null [" << __FILE__ << ":" << __LINE__ << "]"; \
            mbc_test::TestRunner::instance().recordFail(__func__, oss_.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_NULL(ptr) \
    do { \
        if ((ptr) == nullptr) { \
            mbc_test::TestRunner::instance().recordPass(__func__); \
        } else { \
            std::ostringstream oss_; \
            oss_ << #ptr << " is not null [" << __FILE__ << ":" << __LINE__ << "]"; \
            mbc_test::TestRunner::instance().recordFail(__func__, oss_.str()); \
            return; \
        } \
    } while (0)

#endif // __TEST_FRAMEWORK_H__
