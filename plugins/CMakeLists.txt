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

list(SORT PLUGIN_SOURCES)

# Some plugins depend on external libraries.  We currently only need zlib, but
# the structure below makes it straightforward to extend if additional
# third‑party dependencies are introduced later on.
set(ZLIB_PLUGINS lammps lpmd)

find_package(ZLIB REQUIRED)

foreach(plugin_src ${PLUGIN_SOURCES})
    get_filename_component(plugin_name "${plugin_src}" NAME_WE)
    # Use a CMake‑friendly target name (``-`` is not a valid character for
    # targets) while preserving the output file name expected by LPMD at
    # runtime.
    string(REPLACE "-" "_" plugin_target "plugin_${plugin_name}")

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
endforeach()
