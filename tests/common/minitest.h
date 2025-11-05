// 极简测试框架（仅用于本项目示例用）
#pragma once

#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <cmath>

namespace mini {

struct TestCase {
    const char* name;
    void (*func)();
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> r;
    return r;
}

inline void register_test(const char* name, void (*func)()) {
    registry().push_back(TestCase{name, func});
}

inline int run_all() {
    int failed = 0;
    std::cout << "[mini] Running " << registry().size() << " tests" << std::endl;
    for (const auto& t : registry()) {
        try {
            t.func();
            std::cout << "[PASS] " << t.name << std::endl;
        } catch (const std::exception& e) {
            std::cout << "[FAIL] " << t.name << " :: " << e.what() << std::endl;
            ++failed;
        } catch (...) {
            std::cout << "[FAIL] " << t.name << " :: unknown exception" << std::endl;
            ++failed;
        }
    }
    std::cout << "[mini] Failed: " << failed << std::endl;
    return failed;
}

} // namespace mini

#define MINI_CONCAT_INNER(a,b) a##b
#define MINI_CONCAT(a,b) MINI_CONCAT_INNER(a,b)

#define TEST_CASE(NAME) \
    static void MINI_CONCAT(test_fn_, __LINE__)(); \
    static bool MINI_CONCAT(test_reg_, __LINE__) = (mini::register_test(NAME, &MINI_CONCAT(test_fn_, __LINE__)), true); \
    static void MINI_CONCAT(test_fn_, __LINE__)()

#define REQUIRE(COND) do { if (!(COND)) throw std::runtime_error(std::string("REQUIRE failed: ") + #COND); } while(0)
#define REQUIRE_NEAR(A,B,EPS) do { if (std::abs((A)-(B)) > (EPS)) throw std::runtime_error("REQUIRE_NEAR failed: " #A " ~= " #B); } while(0)

#ifdef MINI_TEST_MAIN
int main() { return mini::run_all(); }
#endif
