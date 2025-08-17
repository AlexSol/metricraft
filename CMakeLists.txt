cmake_minimum_required(VERSION 3.20)
project(metricraft-agent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
include(cmake/CPM.cmake)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckTypeSize)
include(FetchContent)

add_executable(metricraft-agent
    src/main.cpp
    src/metrics.hpp
    src/manifest.hpp
    src/plugin.hpp
    src/exec_runner.hpp
)

target_include_directories(metricraft-agent
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# find_package(Boost REQUIRED COMPONENTS process)
# find_package(yaml-cpp REQUIRED)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(json)

CPMAddPackage(
  NAME Boost
  VERSION 1.89.0 # Versions less than 1.85.0 may need patches for installation targets.
  URL https://github.com/boostorg/boost/releases/download/boost-1.89.0/boost-1.89.0-cmake.tar.xz
#   URL_HASH SHA256=7da75f171837577a52bbf217e17f8ea576c7c246e4594d617bfde7fafd408be5
  OPTIONS "BOOST_ENABLE_CMAKE ON" "BOOST_SKIP_INSTALL_RULES ON" # Set `OFF` for installation
          "BUILD_SHARED_LIBS OFF" "BOOST_INCLUDE_LIBRARIES program_options\\\;filesystem\\\;process" # Note the escapes!
)

CPMAddPackage(
  NAME yaml-cpp
  VERSION 0.8.0
  URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.tar.gz
#   URL_HASH SHA256=7da75f171837577a52bbf217e17f8ea576c7c246e4594d617bfde7fafd408be5
#   OPTIONS "BOOST_ENABLE_CMAKE ON" "BOOST_SKIP_INSTALL_RULES ON" # Set `OFF` for installation
#           "BUILD_SHARED_LIBS OFF" "BOOST_INCLUDE_LIBRARIES program_options\\\;filesystem\\\;process" # Note the escapes!
)

target_link_libraries(metricraft-agent
  PRIVATE
    nlohmann_json::nlohmann_json
    yaml-cpp
    pthread
    Boost::process
    Boost::program_options
)