set(LPMLPD_SOURCES
    atom.cc
    atomselector.cc
    cell.cc
    cellformat.cc
    cellgenerator.cc
    cellmanager.cc
    cellreader.cc
    cellwriter.cc
    cmdline.cc
    color.cc
    colorhandler.cc
    configuration.cc
    controlfile.cc
    elements.cc
    error.cc
    integrator.cc
    matrix.cc
    metalpotential.cc
    module.cc
    moduleinfo.cc
    onestepintegrator.cc
    pairpotential.cc
    paramlist.cc
    particleset.cc
    plugin.cc
    pluginmanager.cc
    potential.cc
    properties.cc
    property.cc
    session.cc
    simulation.cc
    simulationbuilder.cc
    simulationhistory.cc
    systemfilter.cc
    systemmixer.cc
    systemmodifier.cc
    timer.cc
    twostepintegrator.cc
    util.cc
    value.cc
    visualizer.cc
)

list(TRANSFORM LPMLPD_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

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
