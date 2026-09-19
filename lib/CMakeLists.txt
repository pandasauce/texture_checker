add_library(texinfo
    src/bmp.cpp
    src/bytes.cpp
    src/dds.cpp
    src/jpeg.cpp
    src/png.cpp
    src/probe.cpp
    src/render.cpp
    src/tga.cpp
)
target_include_directories(texinfo PUBLIC include)
target_link_libraries(texinfo PRIVATE fmt::fmt)
