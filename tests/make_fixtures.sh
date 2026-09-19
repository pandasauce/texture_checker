#!/usr/bin/env zsh
# Regenerates fixtures/generated. Needs ImageMagick on PATH; texconv.exe sits next to this script.
# fixtures/real holds committed real-world files and is never touched here.
set -e
cd "${0:A:h}"
out=fixtures/generated
rm -rf "$out" && mkdir -p "$out"

source=$(mktemp -d)/source.png
magick -size 64x32 plasma:fractal "$source"

dds() {
    local name=$1 format=$2
    shift 2
    ./texconv.exe -nologo -y -f "$format" "$@" -o "$out" "$source" > /dev/null
    mv "$out/${source:t:r}.dds" "$out/$name.dds"
}

# Without -dx10 texconv writes legacy FourCC or legacy uncompressed headers; with it, a DXGI format.
# Both code paths need covering, and the legacy names below are what actually lands on disk.
dds dds_bc1        BC1_UNORM      -m 1
dds dds_bc2        BC2_UNORM      -m 1
dds dds_bc3        BC3_UNORM      -m 1
dds dds_bc4        BC4_UNORM      -m 1
dds dds_bc5        BC5_UNORM      -m 1
dds dds_l8         R8_UNORM       -m 1
dds dds_l8a8       R8G8_UNORM     -m 1
dds dds_bgra8      B8G8R8A8_UNORM -m 1

dds dds_bc1_dx10   BC1_UNORM      -m 1 -dx10
dds dds_bc3_dx10   BC3_UNORM      -m 1 -dx10
dds dds_bc5_dx10   BC5_UNORM      -m 1 -dx10
dds dds_bc6h       BC6H_UF16      -m 1
dds dds_bc7        BC7_UNORM      -m 1
dds dds_r8_dx10    R8_UNORM       -m 1 -dx10
dds dds_rg8_dx10   R8G8_UNORM     -m 1 -dx10
dds dds_rgba8_dx10 R8G8B8A8_UNORM -m 1 -dx10
dds dds_mipped     BC7_UNORM

magick -size 64x32 plasma:fractal -colorspace Gray -type Grayscale        "$out/png_luma.png"
magick -size 64x32 plasma:fractal -define png:color-type=2                "$out/png_rgb.png"
magick -size 64x32 plasma:fractal -colors 64 -type Palette                "$out/png_indexed.png"
magick -size 64x32 plasma:fractal -colorspace Gray -type GrayscaleAlpha -alpha set "$out/png_lumaalpha.png"
magick -size 64x32 plasma:fractal -alpha set -define png:color-type=6     "$out/png_rgba.png"

magick -size 64x32 plasma:fractal                                         "$out/jpg_rgb.jpg"
magick -size 64x32 plasma:fractal -colorspace Gray -type Grayscale        "$out/jpg_luma.jpg"

magick -size 64x32 plasma:fractal -type TrueColor BMP3:"$out/bmp_rgb24.bmp"
magick -size 64x32 plasma:fractal -alpha set -type TrueColorAlpha BMP:"$out/bmp_rgba32.bmp"
magick -size 64x32 plasma:fractal -colors 64 -type Palette BMP3:"$out/bmp_indexed.bmp"

magick -size 64x32 plasma:fractal -type TrueColor tga:"$out/tga_rgb24.tga"
magick -size 64x32 plasma:fractal -alpha set -type TrueColorAlpha tga:"$out/tga_rgba32.tga"
magick -size 64x32 plasma:fractal -colorspace Gray -type Grayscale -depth 8 tga:"$out/tga_luma.tga"
magick -size 64x32 plasma:fractal -type TrueColor -compress RLE tga:"$out/tga_rle.tga"

echo "wrote $(ls "$out" | wc -l) generated fixtures"
