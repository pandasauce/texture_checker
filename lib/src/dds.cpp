#include <fmt/format.h>

#include "parsers.hpp"

namespace texinfo {
namespace {

constexpr std::uint32_t kExpectedHeaderSize = 124;
constexpr std::uint32_t kFlagMipMapCount = 0x20000;

constexpr std::uint32_t kPfAlphaPixels = 0x1;
constexpr std::uint32_t kPfFourCC = 0x4;
constexpr std::uint32_t kPfPaletteIndexed8 = 0x20;
constexpr std::uint32_t kPfRgb = 0x40;
constexpr std::uint32_t kPfLuminance = 0x20000;
constexpr std::uint32_t kPfBumpDuDv = 0x80000;

constexpr std::size_t kOffsetMagic = 0;
constexpr std::size_t kOffsetHeaderSize = 4;
constexpr std::size_t kOffsetFlags = 8;
constexpr std::size_t kOffsetHeight = 12;
constexpr std::size_t kOffsetWidth = 16;
constexpr std::size_t kOffsetMipMapCount = 28;
constexpr std::size_t kOffsetPixelFormatFlags = 80;
constexpr std::size_t kOffsetFourCC = 84;
constexpr std::size_t kOffsetDxgiFormat = 128;

constexpr std::uint32_t fourcc(const char (&tag)[5]) {
    return static_cast<std::uint32_t>(static_cast<unsigned char>(tag[0])) |
           static_cast<std::uint32_t>(static_cast<unsigned char>(tag[1])) << 8 |
           static_cast<std::uint32_t>(static_cast<unsigned char>(tag[2])) << 16 |
           static_cast<std::uint32_t>(static_cast<unsigned char>(tag[3])) << 24;
}

std::string fourcc_to_string(std::uint32_t tag) {
    std::string text;
    for (int shift = 0; shift < 32; shift += 8) {
        const char character = static_cast<char>((tag >> shift) & 0xff);
        text += std::isprint(static_cast<unsigned char>(character)) ? character : '?';
    }
    return text;
}

enum Dxgi : std::uint32_t {
    R32G32B32A32_TYPELESS = 1, R32G32B32A32_FLOAT = 2, R32G32B32A32_UINT = 3, R32G32B32A32_SINT = 4,
    R32G32B32_TYPELESS = 5, R32G32B32_FLOAT = 6, R32G32B32_UINT = 7, R32G32B32_SINT = 8,
    R16G16B16A16_TYPELESS = 9, R16G16B16A16_FLOAT = 10, R16G16B16A16_UNORM = 11,
    R16G16B16A16_UINT = 12, R16G16B16A16_SNORM = 13, R16G16B16A16_SINT = 14,
    R32G32_TYPELESS = 15, R32G32_FLOAT = 16, R32G32_UINT = 17, R32G32_SINT = 18,
    R8G8B8A8_TYPELESS = 27, R8G8B8A8_UNORM = 28, R8G8B8A8_UNORM_SRGB = 29,
    R8G8B8A8_UINT = 30, R8G8B8A8_SNORM = 31, R8G8B8A8_SINT = 32,
    R16G16_TYPELESS = 33, R16G16_FLOAT = 34, R16G16_UNORM = 35,
    R16G16_UINT = 36, R16G16_SNORM = 37, R16G16_SINT = 38,
    R32_FLOAT = 41, R32_UINT = 42, R32_SINT = 43,
    R8G8_TYPELESS = 48, R8G8_UNORM = 49, R8G8_UINT = 50, R8G8_SNORM = 51, R8G8_SINT = 52,
    R16_TYPELESS = 53, R16_FLOAT = 54, R16_UNORM = 56, R16_UINT = 57, R16_SNORM = 58, R16_SINT = 59,
    R8_TYPELESS = 60, R8_UNORM = 61, R8_UINT = 62, R8_SNORM = 63, R8_SINT = 64, A8_UNORM = 65,
    BC1_TYPELESS = 70, BC1_UNORM = 71, BC1_UNORM_SRGB = 72,
    BC2_TYPELESS = 73, BC2_UNORM = 74, BC2_UNORM_SRGB = 75,
    BC3_TYPELESS = 76, BC3_UNORM = 77, BC3_UNORM_SRGB = 78,
    BC4_TYPELESS = 79, BC4_UNORM = 80, BC4_SNORM = 81,
    BC5_TYPELESS = 82, BC5_UNORM = 83, BC5_SNORM = 84,
    B5G6R5_UNORM = 85, B5G5R5A1_UNORM = 86,
    B8G8R8A8_UNORM = 87, B8G8R8X8_UNORM = 88,
    B8G8R8A8_TYPELESS = 90, B8G8R8A8_UNORM_SRGB = 91,
    B8G8R8X8_TYPELESS = 92, B8G8R8X8_UNORM_SRGB = 93,
    BC6H_TYPELESS = 94, BC6H_UF16 = 95, BC6H_SF16 = 96,
    BC7_TYPELESS = 97, BC7_UNORM = 98, BC7_UNORM_SRGB = 99,
    B4G4R4A4_UNORM = 115,
};

struct SurfaceFormat {
    Compression compression;
    Channels channels;
};

SurfaceFormat format_from_fourcc(std::uint32_t tag, bool alpha_pixels) {
    if (tag == fourcc("DXT1")) return {Compression::Bc1, alpha_pixels ? Channels::Rgba : Channels::Rgb};
    if (tag == fourcc("DXT2") || tag == fourcc("DXT3")) return {Compression::Bc2, Channels::Rgba};
    if (tag == fourcc("DXT4") || tag == fourcc("DXT5")) return {Compression::Bc3, Channels::Rgba};
    if (tag == fourcc("ATI1") || tag == fourcc("BC4U") || tag == fourcc("BC4S")) return {Compression::Bc4, Channels::Luma};
    if (tag == fourcc("ATI2") || tag == fourcc("BC5U") || tag == fourcc("BC5S")) return {Compression::Bc5, Channels::Rg};
    throw ProbeError(fmt::format("unsupported DDS FourCC '{}'", fourcc_to_string(tag)));
}

SurfaceFormat format_from_dxgi(std::uint32_t dxgi) {
    switch (dxgi) {
    case BC1_TYPELESS: case BC1_UNORM: case BC1_UNORM_SRGB:
        return {Compression::Bc1, Channels::Rgb};
    case BC2_TYPELESS: case BC2_UNORM: case BC2_UNORM_SRGB:
        return {Compression::Bc2, Channels::Rgba};
    case BC3_TYPELESS: case BC3_UNORM: case BC3_UNORM_SRGB:
        return {Compression::Bc3, Channels::Rgba};
    case BC4_TYPELESS: case BC4_UNORM: case BC4_SNORM:
        return {Compression::Bc4, Channels::Luma};
    case BC5_TYPELESS: case BC5_UNORM: case BC5_SNORM:
        return {Compression::Bc5, Channels::Rg};
    case BC6H_TYPELESS: case BC6H_UF16: case BC6H_SF16:
        return {Compression::Bc6h, Channels::Rgb};
    case BC7_TYPELESS: case BC7_UNORM: case BC7_UNORM_SRGB:
        return {Compression::Bc7, Channels::Rgba};
    case R32G32B32A32_TYPELESS: case R32G32B32A32_FLOAT: case R32G32B32A32_UINT: case R32G32B32A32_SINT:
    case R16G16B16A16_TYPELESS: case R16G16B16A16_FLOAT: case R16G16B16A16_UNORM:
    case R16G16B16A16_UINT: case R16G16B16A16_SNORM: case R16G16B16A16_SINT:
    case R8G8B8A8_TYPELESS: case R8G8B8A8_UNORM: case R8G8B8A8_UNORM_SRGB:
    case R8G8B8A8_UINT: case R8G8B8A8_SNORM: case R8G8B8A8_SINT:
    case B8G8R8A8_UNORM: case B8G8R8A8_TYPELESS: case B8G8R8A8_UNORM_SRGB:
    case B5G5R5A1_UNORM: case B4G4R4A4_UNORM:
        return {Compression::Lossless, Channels::Rgba};
    case R32G32B32_TYPELESS: case R32G32B32_FLOAT: case R32G32B32_UINT: case R32G32B32_SINT:
    case B8G8R8X8_UNORM: case B8G8R8X8_TYPELESS: case B8G8R8X8_UNORM_SRGB: case B5G6R5_UNORM:
        return {Compression::Lossless, Channels::Rgb};
    case R32G32_TYPELESS: case R32G32_FLOAT: case R32G32_UINT: case R32G32_SINT:
    case R16G16_TYPELESS: case R16G16_FLOAT: case R16G16_UNORM:
    case R16G16_UINT: case R16G16_SNORM: case R16G16_SINT:
    case R8G8_TYPELESS: case R8G8_UNORM: case R8G8_UINT: case R8G8_SNORM: case R8G8_SINT:
        return {Compression::Lossless, Channels::Rg};
    case R32_FLOAT: case R32_UINT: case R32_SINT:
    case R16_TYPELESS: case R16_FLOAT: case R16_UNORM: case R16_UINT: case R16_SNORM: case R16_SINT:
    case R8_TYPELESS: case R8_UNORM: case R8_UINT: case R8_SNORM: case R8_SINT: case A8_UNORM:
        return {Compression::Lossless, Channels::Luma};
    default:
        throw ProbeError(fmt::format("unsupported DXGI format {}", dxgi));
    }
}

Channels channels_from_pixel_format(std::uint32_t pixel_format_flags) {
    const bool alpha_pixels = pixel_format_flags & kPfAlphaPixels;
    if (pixel_format_flags & kPfPaletteIndexed8) return Channels::Indexed;
    if (pixel_format_flags & kPfLuminance) return alpha_pixels ? Channels::LumaAlpha : Channels::Luma;
    if (pixel_format_flags & kPfBumpDuDv) return Channels::Rg;
    if (pixel_format_flags & kPfRgb) return alpha_pixels ? Channels::Rgba : Channels::Rgb;
    throw ProbeError(fmt::format("unsupported DDS pixel format flags {:#x}", pixel_format_flags));
}

}

TextureInfo parse_dds(Bytes header) {
    if (read_u32_le(header, kOffsetMagic) != fourcc("DDS ")) {
        throw ProbeError("missing DDS magic bytes");
    }
    if (read_u32_le(header, kOffsetHeaderSize) != kExpectedHeaderSize) {
        throw ProbeError("DDS header size is not 124");
    }

    TextureInfo info{};
    info.container = Container::Dds;
    info.height = read_u32_le(header, kOffsetHeight);
    info.width = read_u32_le(header, kOffsetWidth);

    const std::uint32_t declared_mips = read_u32_le(header, kOffsetMipMapCount);
    const bool has_mip_count = read_u32_le(header, kOffsetFlags) & kFlagMipMapCount;
    info.mip_count = has_mip_count && declared_mips > 0 ? declared_mips : 1;

    const std::uint32_t pixel_format_flags = read_u32_le(header, kOffsetPixelFormatFlags);
    if (pixel_format_flags & kPfFourCC) {
        const std::uint32_t tag = read_u32_le(header, kOffsetFourCC);
        const SurfaceFormat format = tag == fourcc("DX10")
                                         ? format_from_dxgi(read_u32_le(header, kOffsetDxgiFormat))
                                         : format_from_fourcc(tag, pixel_format_flags & kPfAlphaPixels);
        info.compression = format.compression;
        info.channels = format.channels;
    } else {
        info.compression = Compression::Lossless;
        info.channels = channels_from_pixel_format(pixel_format_flags);
    }
    return info;
}

}
