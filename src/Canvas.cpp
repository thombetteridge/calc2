#include "Canvas.hpp"

#include <cstdint>

#include "Font.hpp"

#include "tom.hpp"

Canvas::Canvas(int w, int h)
    : width(w), height(h)
{
    buffer = std::make_unique<Colour[]>(w * h);
}


void Canvas::draw_pixel(int x, int y, Colour colour)
{
    if (x <= 0 or x > width or y <= 0 or y > height)
        return;

    buffer[y * width + x] = colour;
}


void Canvas::clear(Colour colour)
{
    for (int i = 0; i < width * height; i += 1)
        buffer[i] = colour;
}

void Canvas::draw_rect(int x, int y, int w, int h, Colour colour)
{
    for (int j = y; j < y + h; j += 1) {
        for (int i = x; i < x + w; i += 1) {
            draw_pixel(i, j, colour);
        }
    }
}

void Canvas::draw_frame(int x, int y, int w, int h, Colour colour)
{
    let x0 = x;
    let y0 = y;

    let x1 = x + w;
    let y1 = y;

    let x2 = x + w;
    let y2 = y + h;

    let x3 = x;
    let y3 = y + h;


    draw_line(x0, y0, x1, y1, colour);
    draw_line(x1, y1, x2, y2, colour);
    draw_line(x2, y2, x3, y3, colour);
    draw_line(x3, y3, x0, y0, colour);
}

void Canvas::draw_line(int x0, int y0, int x1, int y1, Colour colour)
{
    x0 = std::min(x0, x1);
    y0 = std::min(y0, y1);
    x1 = std::max(x0, x1);
    y1 = std::max(y0, y1);

    let dx = x1 - x0;
    let dy = y1 - y0;

    int D = 2 * dy - dx;
    int y = y0;

    for (int x = x0; x < x1; x += 1) {
        draw_pixel(x, y, colour);
        if (D > 0) {
            y += 1;
            D += 2 * (dy - dx);
        }
        else {
            D += 2 * dy;
        }
    }
}

auto sq = [](auto x) { return x * x; };

void Canvas::draw_circle(int cx, int cy, int radius, Colour colour)
{
    int x0 = cx - radius;
    int x1 = cx + radius;

    int y0 = cy - radius;
    int y1 = cy + radius;

    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            int dx = x - cx;
            int dy = y - cy;
            if (sq(dx) + sq(dy) <= sq(radius)) {
                draw_pixel(x, y, colour);
            }
        }
    }
}


void Canvas::draw_circle_line(int cx, int cy, int radius, Colour colour)
{
    // Source:
    // https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/

    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;


    draw_pixel(cx + x, cy + y, colour);
    draw_pixel(cx - x, cy + y, colour);
    draw_pixel(cx + x, cy - y, colour);
    draw_pixel(cx - x, cy - y, colour);
    draw_pixel(cx + y, cy + x, colour);
    draw_pixel(cx - y, cy + x, colour);
    draw_pixel(cx + y, cy - x, colour);
    draw_pixel(cx - y, cy - x, colour);

    while (y >= x) {
        x += 1;

        if (d > 0) {
            y -= 1;
            d += 4 * (x - y) + 10;
        }
        else {
            d += 4 * x + 6;
        }

        draw_pixel(cx + x, cy + y, colour);
        draw_pixel(cx - x, cy + y, colour);
        draw_pixel(cx + x, cy - y, colour);
        draw_pixel(cx - x, cy - y, colour);
        draw_pixel(cx + y, cy + x, colour);
        draw_pixel(cx - y, cy + x, colour);
        draw_pixel(cx + y, cy - x, colour);
        draw_pixel(cx - y, cy - x, colour);
    }
}


void Canvas::draw_text(Font const & font, char const * str, int len, int x, int y, Colour colour)
{
    float fx = as<float>(x);
    float fy = as<float>(y) + font.baseline;

    for (int i = 0; i < len; ++i) {
        let c = as<uint8_t>(str[i]);
        if (c < 32 || c >= 128)
            continue;

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(font.cdata, font.atlas_w, font.atlas_h,
                           c - 32, &fx, &fy, &q, 1);

        let x0 = as<int>(q.x0);
        let x1 = as<int>(q.x1);
        let y0 = as<int>(q.y0);
        let y1 = as<int>(q.y1);

        let u_step = (q.s1 - q.s0) / as<float>(x1 - x0);
        let v_step = (q.t1 - q.t0) / as<float>(y1 - y0);

        for (int py = y0; py < y1; ++py) {
            for (int px = x0; px < x1; ++px) {
                let u = q.s0 + as<float>(px - x0) * u_step;
                let v = q.t0 + as<float>(py - y0) * v_step;

                let coverage = font.atlas[as<int>(v * as<float>(font.atlas_h)) * font.atlas_w + as<int>(u * as<float>(font.atlas_w))];
                blend_pixel(px, py, colour, coverage);
            }
        }
    }
}

void Canvas::blend_pixel(int x, int y, Colour colour, uint8_t coverage)
{
    if (coverage == 0 || x < 0 || y < 0 || x >= width || y >= height)
        return;
    if (coverage == 255) {
        draw_pixel(x, y, colour);
        return;
    }

    Colour bg = buffer[y * width + x];

    uint8_t const * src = cast<uint8_t const *>(&colour);
    uint8_t const * dst = cast<uint8_t const *>(&bg);
    uint8_t         out[4];
    for (int i = 0; i < 4; ++i)
        out[i] = as<uint8_t>((src[i] * coverage + dst[i] * (255 - coverage)) / 255);

    draw_pixel(x, y, *cast<Colour *>(out));
}