add_library(lpmd_force_constant SHARED
    src/constant_force.cpp
)

target_link_libraries(lpmd_force_constant
    PRIVATE
        lpmd::core
)

target_compile_features(lpmd_force_constant PUBLIC cxx_std_17)

set_target_properties(lpmd_force_constant PROPERTIES
    PREFIX ""
    OUTPUT_NAME "lpmd_force_constant"
)
