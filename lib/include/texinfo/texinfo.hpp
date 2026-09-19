#pragma once

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

namespace texinfo {

enum class Container { Dds, Png, Jpg, Bmp, Tga };

enum class Channels { Luma, LumaAlpha, Rg, Rgb, Rgba, Indexed };

enum class Compression { Lossless, Lossy, Bc1, Bc2, Bc3, Bc4, Bc5, Bc6h, Bc7 };

struct TextureInfo {
    Container container;
    Channels channels;
    Compression compression;
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t mip_count;
    std::uintmax_t size_bytes;
};

class ProbeError : public std::runtime_error {
public:
    explicit ProbeError(const std::string& message) : std::runtime_error(message) {}
};

TextureInfo probe(const std::filesystem::path& file);

std::string_view container_name(Container container);
std::string_view channels_name(Channels channels);
std::string_view compression_name(Compression compression);

std::string resolution_text(const TextureInfo& info);
std::string mip_text(const TextureInfo& info);
std::string size_text(const TextureInfo& info, bool exact_bytes);

}
