cmake_minimum_required(VERSION 3.16)

project(lpmd2 VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
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

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/lpmd2Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/lpmd2Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lpmd2
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/lpmd2ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    EXPORT lpmd2Targets
    FILE lpmd2Targets.cmake
    NAMESPACE lpmd::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lpmd2
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/lpmd2Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/lpmd2ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lpmd2
)
