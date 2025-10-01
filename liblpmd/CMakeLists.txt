add_library(liblpmd
    src/simulation.cpp
    src/plugin_loader.cpp
)

add_library(lpmd::core ALIAS liblpmd)

target_include_directories(liblpmd
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(liblpmd PUBLIC cxx_std_17)

if(WIN32)
    target_compile_definitions(liblpmd PRIVATE NOMINMAX)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(liblpmd PUBLIC dl)
endif()
