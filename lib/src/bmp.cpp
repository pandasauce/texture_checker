#include <cstdlib>

#include <fmt/format.h>

#include "parsers.hpp"

namespace texinfo {
namespace {

constexpr std::size_t kOffsetDibHeaderSize = 14;
constexpr std::size_t kOffsetCoreWidth = 18;
constexpr std::size_t kOffsetCoreHeight = 20;
constexpr std::size_t kOffsetCoreBitCount = 24;
constexpr std::size_t kOffsetInfoWidth = 18;
constexpr std::size_t kOffsetInfoHeight = 22;
constexpr std::size_t kOffsetInfoBitCount = 28;
constexpr std::size_t kOffsetInfoCompression = 30;

constexpr std::uint32_t kCoreHeaderSize = 12;
constexpr std::uint32_t kCompressionJpeg = 4;
constexpr std::uint32_t kCompressionPng = 5;

std::uint32_t absolute_extent(std::uint32_t stored) {
    const std::int32_t signed_extent = static_cast<std::int32_t>(stored);
    return static_cast<std::uint32_t>(std::abs(signed_extent));
}

Channels channels_from_bit_count(std::uint32_t bit_count) {
    if (bit_count <= 8) return Channels::Indexed;
    if (bit_count == 16 || bit_count == 24) return Channels::Rgb;
    if (bit_count == 32) return Channels::Rgba;
    throw ProbeError(fmt::format("unsupported BMP bit count {}", bit_count));
}

}

TextureInfo parse_bmp(Bytes header) {
    if (read_u8(header, 0) != 'B' || read_u8(header, 1) != 'M') {
        throw ProbeError("missing BMP magic bytes");
    }

    TextureInfo info{};
    info.container = Container::Bmp;
    info.compression = Compression::Lossless;
    info.mip_count = 1;

    const std::uint32_t dib_header_size = read_u32_le(header, kOffsetDibHeaderSize);
    if (dib_header_size == kCoreHeaderSize) {
        info.width = read_u16_le(header, kOffsetCoreWidth);
        info.height = read_u16_le(header, kOffsetCoreHeight);
        info.channels = channels_from_bit_count(read_u16_le(header, kOffsetCoreBitCount));
        return info;
    }
    if (dib_header_size < 40) {
        throw ProbeError(fmt::format("unsupported BMP DIB header size {}", dib_header_size));
    }

    const std::uint32_t compression = read_u32_le(header, kOffsetInfoCompression);
    if (compression == kCompressionJpeg || compression == kCompressionPng) {
        throw ProbeError("BMP with an embedded JPEG or PNG payload is not supported");
    }
    info.width = absolute_extent(read_u32_le(header, kOffsetInfoWidth));
    info.height = absolute_extent(read_u32_le(header, kOffsetInfoHeight));
    info.channels = channels_from_bit_count(read_u16_le(header, kOffsetInfoBitCount));
    return info;
}

}
