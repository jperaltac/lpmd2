add_library(lpunit STATIC
    test.cc
    testsuite.cc
)

add_library(lpmd::lpunit ALIAS lpunit)

set_target_properties(lpunit PROPERTIES OUTPUT_NAME lpunit)

target_include_directories(lpunit
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_features(lpunit PUBLIC cxx_std_17)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/lpmaketest.in
    ${CMAKE_CURRENT_BINARY_DIR}/lpmaketest.py
    COPYONLY
)

set(LPMD_LPUNIT_GENERATOR ${CMAKE_CURRENT_BINARY_DIR}/lpmaketest.py PARENT_SCOPE)
