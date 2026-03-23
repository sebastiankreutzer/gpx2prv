include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
  GIT_SHALLOW TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(test_gpx test_gpx.cpp)
target_link_libraries(test_gpx PRIVATE gpx2prv_lib gtest_main)
target_compile_definitions(test_gpx PRIVATE TESTS_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
gtest_discover_tests(test_gpx)

add_executable(test_prv test_prv.cpp)
target_link_libraries(test_prv PRIVATE gpx2prv_lib gtest_main)
target_compile_definitions(test_prv PRIVATE TESTS_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
gtest_discover_tests(test_prv)
