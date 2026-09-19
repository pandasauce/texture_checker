add_executable(texcheck
    glob.cpp
    main.cpp
)
target_link_libraries(texcheck PRIVATE texinfo argparse::argparse fmt::fmt spdlog::spdlog)
