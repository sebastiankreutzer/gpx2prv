cmake_minimum_required(VERSION 3.10)
project(gpx2prv VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  pugixml
  GIT_REPOSITORY https://github.com/zeux/pugixml.git
  GIT_TAG v1.14
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(pugixml)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(gpx2prv_lib STATIC src/gpx.cpp src/prv.cpp)
target_include_directories(gpx2prv_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(gpx2prv_lib PUBLIC pugixml::pugixml)

add_executable(gpx2prv src/main.cpp)
target_link_libraries(gpx2prv PRIVATE gpx2prv_lib)

enable_testing()
add_subdirectory(tests)

install(TARGETS gpx2prv gpx2prv_lib DESTINATION lib)
install(DIRECTORY include/gpx2prv DESTINATION include)
