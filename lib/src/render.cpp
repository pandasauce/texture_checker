#include <array>

#include <fmt/format.h>

#include "texinfo/texinfo.hpp"

namespace texinfo {

std::string_view container_name(Container container) {
    switch (container) {
    case Container::Dds: return "DDS";
    case Container::Png: return "PNG";
    case Container::Jpg: return "JPG";
    case Container::Bmp: return "BMP";
    case Container::Tga: return "TGA";
    }
    return "?";
}

std::string_view channels_name(Channels channels) {
    switch (channels) {
    case Channels::Luma: return "Luma";
    case Channels::LumaAlpha: return "Luma+Alpha";
    case Channels::Rg: return "RG";
    case Channels::Rgb: return "RGB";
    case Channels::Rgba: return "RGBA";
    case Channels::Indexed: return "Indexed";
    }
    return "?";
}

std::string_view compression_name(Compression compression) {
    switch (compression) {
    case Compression::Lossless: return "lossless";
    case Compression::Lossy: return "lossy";
    case Compression::Bc1: return "BC1";
    case Compression::Bc2: return "BC2";
    case Compression::Bc3: return "BC3";
    case Compression::Bc4: return "BC4";
    case Compression::Bc5: return "BC5";
    case Compression::Bc6h: return "BC6H";
    case Compression::Bc7: return "BC7";
    }
    return "?";
}

std::string resolution_text(const TextureInfo& info) {
    return fmt::format("{}x{}", info.width, info.height);
}

std::string mip_text(const TextureInfo& info) {
    return info.mip_count > 1 ? fmt::format("{} mips", info.mip_count) : "no mips";
}

std::string size_text(const TextureInfo& info, bool exact_bytes) {
    if (exact_bytes) {
        return fmt::format("{}", info.size_bytes);
    }
    constexpr std::array<std::string_view, 5> kUnits = {"B", "KiB", "MiB", "GiB", "TiB"};
    double amount = static_cast<double>(info.size_bytes);
    std::size_t unit = 0;
    while (amount >= 1024.0 && unit + 1 < kUnits.size()) {
        amount /= 1024.0;
        ++unit;
    }
    return unit == 0 ? fmt::format("{} B", info.size_bytes) : fmt::format("{:.1f} {}", amount, kUnits[unit]);
}

}
