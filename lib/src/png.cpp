#include <algorithm>
#include <array>

#include <fmt/format.h>

#include "parsers.hpp"

namespace texinfo {
namespace {

constexpr std::array<std::uint8_t, 8> kSignature = {0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a};
constexpr std::array<std::uint8_t, 4> kIhdrType = {'I', 'H', 'D', 'R'};

constexpr std::size_t kOffsetIhdrType = 12;
constexpr std::size_t kOffsetWidth = 16;
constexpr std::size_t kOffsetHeight = 20;
constexpr std::size_t kOffsetColourType = 25;

Channels channels_from_colour_type(std::uint8_t colour_type) {
    switch (colour_type) {
    case 0: return Channels::Luma;
    case 2: return Channels::Rgb;
    case 3: return Channels::Indexed;
    case 4: return Channels::LumaAlpha;
    case 6: return Channels::Rgba;
    default: throw ProbeError(fmt::format("unsupported PNG colour type {}", colour_type));
    }
}

}

TextureInfo parse_png(Bytes header) {
    if (header.size() < kSignature.size() ||
        !std::equal(kSignature.begin(), kSignature.end(), header.begin())) {
        throw ProbeError("missing PNG signature");
    }
    if (header.size() <= kOffsetColourType) {
        throw ProbeError("header is truncated");
    }
    const Bytes chunk_type = header.subspan(kOffsetIhdrType, kIhdrType.size());
    if (!std::equal(kIhdrType.begin(), kIhdrType.end(), chunk_type.begin())) {
        throw ProbeError("first PNG chunk is not IHDR");
    }

    TextureInfo info{};
    info.container = Container::Png;
    info.compression = Compression::Lossless;
    info.width = read_u32_be(header, kOffsetWidth);
    info.height = read_u32_be(header, kOffsetHeight);
    info.mip_count = 1;
    info.channels = channels_from_colour_type(read_u8(header, kOffsetColourType));
    return info;
}

}
