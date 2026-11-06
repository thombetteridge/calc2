#pragma once

#include <cstdint>
#include <vector>


#include "tom.hpp"

struct Font;

struct Canvas {
    using Colour = uint32_t;

    std::vector<Colour> buffer;
    int                 width {};
    int                 height {};

    Canvas(int w, int h);
    void resize(int w, int h);

    void clear(Colour colour);
    void draw_pixel(int x, int y, Colour colour);
    void draw_rect(int x, int y, int w, int h, Colour colour);
    void draw_frame(int x, int y, int w, int h, Colour colour);
    void draw_line(int x0, int y0, int x1, int y1, Colour colour);
    void draw_circle(int cx, int cy, int radius, Colour colour);
    void draw_circle_line(int cx, int cy, int radius, Colour colour);
    void draw_text(Font const & font, char const * ptr, int len, int x, int y, Colour colour);
    void blend_pixel(int x, int y, Colour colour, uint8_t coverage);
};

inline constexpr Canvas::Colour rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return as<uint32_t>(r) | (as<uint32_t>(g) << 8u) | (as<uint32_t>(b) << 16u) | (as<uint32_t>(a) << 24u);
}

inline constexpr Canvas::Colour rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return rgba(r, g, b, 255);
}

constexpr Canvas::Colour colour_white   = rgb(255, 255, 255);
constexpr Canvas::Colour colour_black   = rgb(0, 0, 0);
constexpr Canvas::Colour colour_gray    = rgb(200, 200, 200);
constexpr Canvas::Colour colour_blue    = rgb(0, 0, 255);
constexpr Canvas::Colour colour_red     = rgb(255, 0, 0);
constexpr Canvas::Colour colour_green   = rgb(0, 255, 0);
constexpr Canvas::Colour colour_cyan    = rgb(0, 255, 255);
constexpr Canvas::Colour colour_magenta = rgb(255, 0, 255);
constexpr Canvas::Colour colour_yellow  = rgb(255, 255, 0);
