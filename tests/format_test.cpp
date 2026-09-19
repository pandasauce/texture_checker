#include <filesystem>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "texinfo/texinfo.hpp"

namespace {

struct Datasets {
    std::string format;
    std::string channels;
    std::string compression;
    std::string resolution;
    std::string mips;
};

texinfo::TextureInfo check(const std::string& fixture, const Datasets& expected) {
    const std::filesystem::path file = std::filesystem::path(TEXINFO_FIXTURE_DIR) / fixture;
    INFO("fixture: " << fixture);
    const texinfo::TextureInfo info = texinfo::probe(file);
    CHECK(texinfo::container_name(info.container) == expected.format);
    CHECK(texinfo::channels_name(info.channels) == expected.channels);
    CHECK(texinfo::compression_name(info.compression) == expected.compression);
    CHECK(texinfo::resolution_text(info) == expected.resolution);
    CHECK(texinfo::mip_text(info) == expected.mips);
    CHECK(info.size_bytes == std::filesystem::file_size(file));
    CHECK(texinfo::size_text(info, true) == std::to_string(info.size_bytes));
    return info;
}

}

TEST_CASE("DDS datasets") {
    check("generated/dds_bc1.dds", {"DDS", "RGB", "BC1", "64x32", "no mips"});
    check("generated/dds_bc2.dds", {"DDS", "RGBA", "BC2", "64x32", "no mips"});
    check("generated/dds_bc3.dds", {"DDS", "RGBA", "BC3", "64x32", "no mips"});
    check("generated/dds_bc4.dds", {"DDS", "Luma", "BC4", "64x32", "no mips"});
    check("generated/dds_bc5.dds", {"DDS", "RG", "BC5", "64x32", "no mips"});
    check("generated/dds_l8.dds", {"DDS", "Luma", "lossless", "64x32", "no mips"});
    check("generated/dds_l8a8.dds", {"DDS", "Luma+Alpha", "lossless", "64x32", "no mips"});
    check("generated/dds_bgra8.dds", {"DDS", "RGBA", "lossless", "64x32", "no mips"});

    check("generated/dds_bc1_dx10.dds", {"DDS", "RGB", "BC1", "64x32", "no mips"});
    check("generated/dds_bc3_dx10.dds", {"DDS", "RGBA", "BC3", "64x32", "no mips"});
    check("generated/dds_bc5_dx10.dds", {"DDS", "RG", "BC5", "64x32", "no mips"});
    check("generated/dds_bc6h.dds", {"DDS", "RGB", "BC6H", "64x32", "no mips"});
    check("generated/dds_bc7.dds", {"DDS", "RGBA", "BC7", "64x32", "no mips"});
    check("generated/dds_r8_dx10.dds", {"DDS", "Luma", "lossless", "64x32", "no mips"});
    check("generated/dds_rg8_dx10.dds", {"DDS", "RG", "lossless", "64x32", "no mips"});
    check("generated/dds_rgba8_dx10.dds", {"DDS", "RGBA", "lossless", "64x32", "no mips"});

    check("generated/dds_mipped.dds", {"DDS", "RGBA", "BC7", "64x32", "7 mips"});
}

TEST_CASE("PNG datasets") {
    check("generated/png_luma.png", {"PNG", "Luma", "lossless", "64x32", "no mips"});
    check("generated/png_lumaalpha.png", {"PNG", "Luma+Alpha", "lossless", "64x32", "no mips"});
    check("generated/png_rgb.png", {"PNG", "RGB", "lossless", "64x32", "no mips"});
    check("generated/png_rgba.png", {"PNG", "RGBA", "lossless", "64x32", "no mips"});
    check("generated/png_indexed.png", {"PNG", "Indexed", "lossless", "64x32", "no mips"});
}

TEST_CASE("JPG datasets") {
    check("generated/jpg_luma.jpg", {"JPG", "Luma", "lossy", "64x32", "no mips"});
    check("generated/jpg_rgb.jpg", {"JPG", "RGB", "lossy", "64x32", "no mips"});
}

TEST_CASE("BMP datasets") {
    check("generated/bmp_rgb24.bmp", {"BMP", "RGB", "lossless", "64x32", "no mips"});
    check("generated/bmp_rgba32.bmp", {"BMP", "RGBA", "lossless", "64x32", "no mips"});
    check("generated/bmp_indexed.bmp", {"BMP", "Indexed", "lossless", "64x32", "no mips"});
}

TEST_CASE("TGA datasets") {
    check("generated/tga_luma.tga", {"TGA", "Luma", "lossless", "64x32", "no mips"});
    check("generated/tga_rgb24.tga", {"TGA", "RGB", "lossless", "64x32", "no mips"});
    check("generated/tga_rgba32.tga", {"TGA", "RGBA", "lossless", "64x32", "no mips"});
    check("generated/tga_rle.tga", {"TGA", "RGB", "lossless", "64x32", "no mips"});

    check("real/Shared_Tire_Falken_Azenis_RT615K_Large_Blur_NO.tga",
          {"TGA", "RGBA", "lossless", "512x512", "no mips"});
    const texinfo::TextureInfo tire =
        check("real/Shared_Tire_Falken_Azenis_RT615K_Large_CS.tga",
              {"TGA", "RGBA", "lossless", "1024x1024", "no mips"});
    CHECK(texinfo::size_text(tire, false) == "4.0 MiB");
}
