find_package(Python3 REQUIRED COMPONENTS Interpreter)

if(NOT LPMD_LPUNIT_GENERATOR)
    message(FATAL_ERROR "LPUnit generator path not provided")
endif()

set(LPMD_TEST_UNITS
    arraytest.unit
    atomselector.unit
    atomtest.unit
    cmdline.unit
    colorhandler.unit
    configurationtest.unit
    controlfile.unit
    fixedsizeparticlesettest.unit
    manipulations.unit
    nonorthogonalcelltest.unit
    orthogonalcelltest.unit
    particlesettest.unit
    propertiestest.unit
    simulationcelltest.unit
    simulationtest.unit
    steppertest.unit
    taghandler.unit
    tasktest.unit
    utiltest.unit
    vectortest.unit
)

set(LPMD_DISABLED_TESTS)
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../lpmd/simulationcell.h)
    list(APPEND LPMD_DISABLED_TESTS simulationcelltest.unit)
endif()

foreach(disabled ${LPMD_DISABLED_TESTS})
    list(REMOVE_ITEM LPMD_TEST_UNITS ${disabled})
    message(WARNING "Skipping ${disabled} because required headers are unavailable")
endforeach()

set(LPMD_TEST_TARGETS)

foreach(unit ${LPMD_TEST_UNITS})
    get_filename_component(test_name ${unit} NAME_WE)
    set(generated_source ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.cc)

    add_custom_command(
        OUTPUT ${generated_source}
        COMMAND ${Python3_EXECUTABLE} ${LPMD_LPUNIT_GENERATOR} ${CMAKE_CURRENT_SOURCE_DIR}/${unit} ${generated_source}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${unit} ${LPMD_LPUNIT_GENERATOR}
        COMMENT "Generating ${test_name} unit test source"
        VERBATIM
    )

    set_source_files_properties(${generated_source} PROPERTIES GENERATED TRUE)

    add_executable(${test_name}_unit ${generated_source})
    target_link_libraries(${test_name}_unit PRIVATE lpunit lpmd::core)
    add_test(NAME lpmd.${test_name} COMMAND ${test_name}_unit)

    list(APPEND LPMD_TEST_TARGETS ${test_name}_unit)
endforeach()

if(LPMD_TEST_TARGETS)
    add_custom_target(liblpmd_unit_tests DEPENDS ${LPMD_TEST_TARGETS})
endif()
