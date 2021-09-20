#pragma once
#ifdef MERGE_TEST
#include <vector>

struct MergeTests {
    static inline std::vector<void (*)()> setupTests;
    static inline std::vector<void (*)()> loadTests;

    static void SetupTests() {
        for (auto setup : setupTests) {
            setup();
        }
    }

    static void LoadTests() {
        for (auto load : loadTests) {
            load();
        }
    }
};

#define TEST_SETUP(name)                                                       \
    void Setup##name##Test();                                                  \
    struct __Setup##name##Test_Struct {                                        \
        __Setup##name##Test_Struct() {                                         \
            MergeTests::setupTests.push_back(Setup##name##Test);               \
        }                                                                      \
    };                                                                         \
    static __Setup##name##Test_Struct __Setup##name##Test_Instance;            \
    void Setup##name##Test()

#define TEST_LOAD(name)                                                        \
    void Load##name##Test();                                                   \
    struct __Load##name##Test_Struct {                                         \
        __Load##name##Test_Struct() {                                          \
            MergeTests::loadTests.push_back(Load##name##Test);                \
        }                                                                      \
    };                                                                         \
    static __Load##name##Test_Struct __Load##name##Test_Instance;              \
    void Load##name##Test()

#endif
