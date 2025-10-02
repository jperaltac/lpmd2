cmake_minimum_required(VERSION 3.16)

project(lpmd2 VERSION 0.1.0 LANGUAGES CXX)
# Provide a sensible default build type for single-config generators
if(NOT CMAKE_CONFIGURATION_TYPES)
    set(_LPMD_BUILD_TYPES Debug Release RelWithDebInfo MinSizeRel)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING
            "Build type (choices: Debug, Release, RelWithDebInfo, MinSizeRel)." FORCE)
        message(STATUS "No build type selected, defaulting to Release")
    else()
        set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
            "Build type (choices: Debug, Release, RelWithDebInfo, MinSizeRel)." FORCE)
    endif()
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${_LPMD_BUILD_TYPES})
    unset(_LPMD_BUILD_TYPES)
endif()

include(CTest)
option(LPMD_ENABLE_TESTS "Build lpmd unit tests" ${BUILD_TESTING})
if(LPMD_ENABLE_TESTS)
    enable_testing()
endif()

# Organize build outputs in predictable locations
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(liblpmd)
add_subdirectory(plugins)
add_subdirectory(lpmd)
