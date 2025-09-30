include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

get_filename_component(TESTS_DIR "${PROJECT_SOURCE_DIR}/tests/tests" ABSOLUTE)
get_filename_component(KEYS_DIR  "${PROJECT_SOURCE_DIR}/tests/keys"  ABSOLUTE)

add_executable(gtests gtests.cpp)
target_include_directories(gtests PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_definitions(gtests PRIVATE
    TESTS_DIR="${TESTS_DIR}"
    KEYS_DIR="${KEYS_DIR}"
)

target_link_libraries(
    gtests  
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(gtests)