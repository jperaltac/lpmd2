set(LPMD_CORE_SOURCES
    core/atom.cc
    core/atomselector.cc
    core/cell.cc
    core/cellmanager.cc
    core/configuration.cc
    core/matrix.cc
    core/paramlist.cc
    core/particleset.cc
    core/properties.cc
    core/property.cc
    core/value.cc
)

set(LPMD_IO_SOURCES
    io/cellformat.cc
    io/cellreader.cc
    io/cellwriter.cc
    io/controlfile.cc
    io/simulationhistory.cc
)

set(LPMD_PLUGIN_SOURCES
    plugins/module.cc
    plugins/moduleinfo.cc
    plugins/plugin.cc
    plugins/pluginmanager.cc
)

set(LPMD_RUNTIME_SOURCES
    runtime/cmdline.cc
    runtime/session.cc
)

set(LPMD_SIMULATION_SOURCES
    simulation/cellgenerator.cc
    simulation/integrator.cc
    simulation/metalpotential.cc
    simulation/onestepintegrator.cc
    simulation/pairpotential.cc
    simulation/potential.cc
    simulation/simulation.cc
    simulation/simulationbuilder.cc
    simulation/systemfilter.cc
    simulation/systemmixer.cc
    simulation/systemmodifier.cc
    simulation/twostepintegrator.cc
)

set(LPMD_SUPPORT_SOURCES
    support/color.cc
    support/colorhandler.cc
    support/elements.cc
    support/error.cc
    support/timer.cc
    support/util.cc
    support/visualizer.cc
)

set(LPMLPD_SOURCES
    ${LPMD_CORE_SOURCES}
    ${LPMD_IO_SOURCES}
    ${LPMD_PLUGIN_SOURCES}
    ${LPMD_RUNTIME_SOURCES}
    ${LPMD_SIMULATION_SOURCES}
    ${LPMD_SUPPORT_SOURCES}
)

list(TRANSFORM LPMLPD_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${LPMLPD_SOURCES})

set(LPMD_DEFAULT_PLUGIN_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
if(NOT LPMD_DEFAULT_PLUGIN_PATH)
    set(LPMD_DEFAULT_PLUGIN_PATH "${CMAKE_BINARY_DIR}/lib")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    @ONLY
)

add_library(lpmd_core ${LPMLPD_SOURCES})
add_library(lpmd::core ALIAS lpmd_core)

set_target_properties(lpmd_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(lpmd_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_features(lpmd_core PUBLIC cxx_std_17)

if(UNIX AND NOT APPLE)
    target_link_libraries(lpmd_core PRIVATE dl)
endif()
