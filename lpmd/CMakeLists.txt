add_executable(lpmd
    src/main.cpp
)

target_link_libraries(lpmd
    PRIVATE
        lpmd::core
)

target_compile_features(lpmd PUBLIC cxx_std_17)

if(UNIX AND NOT APPLE)
    target_link_libraries(lpmd PRIVATE dl)
endif()
