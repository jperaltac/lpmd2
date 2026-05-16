# Build each plugin in this directory as a loadable module.
#
# The legacy build system relied on a bespoke Python script that manually
# compiled the plugins.  Switching to CMake keeps the workflow consistent with
# the rest of the project and allows us to catch compilation problems much more
# easily.  Every ``*.cc`` file in this folder becomes a plugin module whose
# output name matches the original file name.  We intentionally keep the
# original names (including dashes) because the runtime plugin loader expects
# ``<plugin>.so`` files.

cmake_minimum_required(VERSION 3.16)

# Collect all plugin implementation files that live directly in this folder.
# The sources do not contain additional helper translation units, so a simple
# glob is sufficient and keeps the list in sync when new plugins are added.
file(GLOB PLUGIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
list(REMOVE_ITEM PLUGIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lpvisual_headless.cc")

list(SORT PLUGIN_SOURCES)

# Some plugins depend on external libraries.  We currently only need zlib, but
# the structure below makes it straightforward to extend if additional
# third‑party dependencies are introduced later on.
set(ZLIB_PLUGINS lammps lpmd)
set(OPENGL_PLUGINS display lpvisual)

find_package(ZLIB REQUIRED)
find_package(OpenGL)
find_package(GLUT)


if(LPMD_ENABLE_RUST_HOTSPOTS)
    find_program(CARGO_EXECUTABLE cargo REQUIRED)
    set(LPMD_RUST_DIR "${CMAKE_SOURCE_DIR}/liblpmd/rust")
    set(LPMD_RUST_CDYLIB "${LPMD_RUST_DIR}/target/release/${CMAKE_SHARED_LIBRARY_PREFIX}lpmd_stats${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set(LPMD_RUST_BUILD_LIB "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_LIBRARY_PREFIX}lpmd_stats${CMAKE_SHARED_LIBRARY_SUFFIX}")

    add_custom_command(
        OUTPUT "${LPMD_RUST_BUILD_LIB}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
        COMMAND "${CARGO_EXECUTABLE}" build --release --manifest-path "${LPMD_RUST_DIR}/Cargo.toml"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${LPMD_RUST_CDYLIB}" "${LPMD_RUST_BUILD_LIB}"
        DEPENDS
            "${LPMD_RUST_DIR}/Cargo.toml"
            "${LPMD_RUST_DIR}/Cargo.lock"
            "${LPMD_RUST_DIR}/src/lib.rs"
            "${LPMD_RUST_DIR}/include/lpmd/rust_hotspots.h"
        COMMENT "Building experimental Rust hotspot helpers"
        VERBATIM
    )
    add_custom_target(lpmd_rust_hotspots DEPENDS "${LPMD_RUST_BUILD_LIB}")
endif()

set(HAVE_OPENGL OFF)
if(OpenGL_FOUND AND GLUT_FOUND)
    set(HAVE_OPENGL ON)
endif()

foreach(plugin_src ${PLUGIN_SOURCES})
    get_filename_component(plugin_name "${plugin_src}" NAME_WE)
    # Use a CMake‑friendly target name (``-`` is not a valid character for
    # targets) while preserving the output file name expected by LPMD at
    # runtime.
    string(REPLACE "-" "_" plugin_target "plugin_${plugin_name}")

    if(plugin_name IN_LIST OPENGL_PLUGINS AND NOT HAVE_OPENGL)
        if(plugin_name STREQUAL "lpvisual")
            message(WARNING "Building headless '${plugin_name}' fallback: OpenGL/GLUT headers not found")
            add_library(${plugin_target} MODULE "${CMAKE_CURRENT_SOURCE_DIR}/lpvisual_headless.cc")
            target_link_libraries(${plugin_target} PRIVATE lpmd::core)
            target_compile_features(${plugin_target} PRIVATE cxx_std_17)
            set_target_properties(${plugin_target}
                PROPERTIES
                    PREFIX ""
                    OUTPUT_NAME "${plugin_name}"
            )
        else()
            message(WARNING "Skipping '${plugin_name}' plugin: OpenGL/GLUT headers not found")
        endif()
        continue()
    endif()

    add_library(${plugin_target} MODULE "${plugin_src}")
    target_link_libraries(${plugin_target} PRIVATE lpmd::core)
    target_compile_features(${plugin_target} PRIVATE cxx_std_17)

    # Ensure the produced shared object is ``<name>.so`` rather than the
    # default ``lib<name>.so``.
    set_target_properties(${plugin_target}
        PROPERTIES
            PREFIX ""
            OUTPUT_NAME "${plugin_name}"
    )

    if(plugin_name IN_LIST ZLIB_PLUGINS)
        target_link_libraries(${plugin_target} PRIVATE ZLIB::ZLIB)
    endif()

    if(plugin_name IN_LIST OPENGL_PLUGINS)
        target_link_libraries(${plugin_target} PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
    endif()

    if(LPMD_ENABLE_RUST_HOTSPOTS AND plugin_name STREQUAL "minimumimage")
        add_dependencies(${plugin_target} lpmd_rust_hotspots)
        target_compile_definitions(${plugin_target} PRIVATE LPMD_ENABLE_RUST_HOTSPOTS=1)
        target_include_directories(${plugin_target} PRIVATE "${LPMD_RUST_DIR}/include")
        target_link_libraries(${plugin_target} PRIVATE "${LPMD_RUST_BUILD_LIB}")
        if(UNIX AND NOT APPLE)
            set_property(TARGET ${plugin_target} APPEND PROPERTY BUILD_RPATH "\$ORIGIN")
        endif()
    endif()
endforeach()
