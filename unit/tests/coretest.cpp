#include "../catch.hpp"

#include "geometrize/bitmap/bitmap.h"
#include "geometrize/bitmap/rgba.h"
#include "geometrize/core.h"

geometrize::rgba computeScanlinesColor(const geometrize::rgba targetColor, const geometrize::rgba currentColor, const int alpha)
{
    geometrize::Bitmap target(10, 10, targetColor);
    geometrize::Bitmap current(10, 10, currentColor);
    std::vector<geometrize::Scanline> lines;
    for(int y = 0; y < 10; y++) {
        lines.push_back(geometrize::Scanline(y, 0, 10, 0));
    }
    return geometrize::core::computeColor(target, current, lines, alpha);
}

TEST_CASE("Test computation of scanline color for whole image", "[core]")
{
    REQUIRE(1 != 0);
}

TEST_CASE("Test drawing of scanlines", "[core]")
{
    geometrize::Bitmap image(50, 50, geometrize::rgba{10, 20, 30, 128});
    std::vector<geometrize::Scanline> lines;
    for(int y = 0; y < 50; y++) {
        lines.push_back(geometrize::Scanline(y, 0, 50, 255));
    }

    // TODO check drawn lines
}

TEST_CASE("Test copying of scanlines", "[core]")
{

}

TEST_CASE("Test calculation of average color", "[core]")
{

}