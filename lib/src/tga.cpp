#include <fmt/format.h>

#include "parsers.hpp"

namespace texinfo {
namespace {

constexpr std::size_t kOffsetImageType = 2;
constexpr std::size_t kOffsetWidth = 12;
constexpr std::size_t kOffsetHeight = 14;
constexpr std::size_t kOffsetPixelDepth = 16;
constexpr std::size_t kOffsetDescriptor = 17;

constexpr std::uint8_t kTypeIndexed = 1;
constexpr std::uint8_t kTypeTrueColour = 2;
constexpr std::uint8_t kTypeGreyscale = 3;
constexpr std::uint8_t kRunLengthFlag = 8;

Channels channels_from_type(std::uint8_t image_type, std::uint32_t depth, std::uint8_t alpha_bits) {
    switch (image_type & ~kRunLengthFlag) {
    case kTypeIndexed:
        return Channels::Indexed;
    case kTypeGreyscale:
        return depth == 16 ? Channels::LumaAlpha : Channels::Luma;
    case kTypeTrueColour:
        if (depth == 32) return Channels::Rgba;
        if (depth == 24) return Channels::Rgb;
        return alpha_bits > 0 ? Channels::Rgba : Channels::Rgb;
    default:
        throw ProbeError(fmt::format("unsupported TGA image type {}", image_type));
    }
}

// TGA has no magic bytes, so a plausible header is the only evidence the extension was not a lie.
bool looks_like_tga(std::uint8_t image_type, std::uint32_t width, std::uint32_t height, std::uint32_t depth) {
    const std::uint8_t base_type = image_type & ~kRunLengthFlag;
    const bool known_type = base_type == kTypeIndexed || base_type == kTypeTrueColour || base_type == kTypeGreyscale;
    const bool known_depth = depth == 8 || depth == 15 || depth == 16 || depth == 24 || depth == 32;
    return known_type && known_depth && width > 0 && height > 0;
}

}

TextureInfo parse_tga(Bytes header) {
    const std::uint8_t image_type = read_u8(header, kOffsetImageType);
    const std::uint32_t width = read_u16_le(header, kOffsetWidth);
    const std::uint32_t height = read_u16_le(header, kOffsetHeight);
    const std::uint32_t depth = read_u8(header, kOffsetPixelDepth);
    if (!looks_like_tga(image_type, width, height, depth)) {
        throw ProbeError("header is not a plausible TGA");
    }

    TextureInfo info{};
    info.container = Container::Tga;
    info.compression = Compression::Lossless;
    info.mip_count = 1;
    info.width = width;
    info.height = height;
    info.channels = channels_from_type(image_type, depth, read_u8(header, kOffsetDescriptor) & 0x0f);
    return info;
}

}
