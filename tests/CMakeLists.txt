enable_testing()

find_package(GTest REQUIRED)

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
)

include(GoogleTest)
gtest_discover_tests(gtests)