set(LPMD_APP_VERSION "${PROJECT_VERSION}")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    @ONLY
)

set(LPMD_UTILITY_SOURCES
    application.cc
    autotest.cc
    controlparser.cc
    help.cc
    palmtree.cc
    quickmode.cc
    refsimulation.cc
    replayintegrator.cc
)

add_library(lpmd_app_support STATIC ${LPMD_UTILITY_SOURCES})
add_library(lpmd::app_support ALIAS lpmd_app_support)

target_link_libraries(lpmd_app_support
    PUBLIC
        lpmd::core
)

target_include_directories(lpmd_app_support
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_features(lpmd_app_support PUBLIC cxx_std_17)

set(LPMD_APPS
    lpmd
    lpmd-analyzer
    lpmd-converter
    lpmd-visualizer
)

foreach(app_name ${LPMD_APPS})
    if(app_name STREQUAL "lpmd")
        set(app_source lpmd.cc)
    elseif(app_name STREQUAL "lpmd-analyzer")
        set(app_source analyzer.cc)
    elseif(app_name STREQUAL "lpmd-converter")
        set(app_source converter.cc)
    elseif(app_name STREQUAL "lpmd-visualizer")
        set(app_source visualizer.cc)
    else()
        message(FATAL_ERROR "Unknown application ${app_name}")
    endif()

    string(REPLACE "-" "_" target_name "${app_name}")
    if(app_name STREQUAL "lpmd")
        set(target_name "lpmd")
    endif()

    add_executable(${target_name} ${app_source})
    target_link_libraries(${target_name} PRIVATE lpmd::app_support)

    if(NOT app_name STREQUAL "lpmd")
        set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "${app_name}")
    endif()
endforeach()

set(LPMD_PYTHON_BIN_SCRIPTS
    lpmd-plotter.py
)

foreach(script ${LPMD_PYTHON_BIN_SCRIPTS})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${script}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${script}
        COPYONLY
    )
endforeach()

set(LPMD_PYTHON_PACKAGE_SCRIPTS
    lpplotter/__init__.py
    lpplotter/makeframes.py
    lpplotter/parselpmd2.py
    lpplotter/povscene.py
)

if(LPMD_PYTHON_PACKAGE_SCRIPTS)
    file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lpplotter)
    foreach(script ${LPMD_PYTHON_PACKAGE_SCRIPTS})
        get_filename_component(script_name "${script}" NAME)
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/${script}
            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lpplotter/${script_name}
            COPYONLY
        )
    endforeach()
endif()
