add_executable(texinfo_tests
    format_test.cpp
    smoke_test.cpp
)
target_link_libraries(texinfo_tests PRIVATE texinfo Catch2::Catch2WithMain)
target_compile_definitions(texinfo_tests PRIVATE TEXINFO_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")

include(Catch)
catch_discover_tests(texinfo_tests)
