#include "Font.hpp"

#include "tom.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "embedded_font.hpp"
auto const ttf_data = JetBrainsMono_Regular_ttf;


Font::Font(float pixel_height)
{
    auto const w = 512;
    auto const h = 512;
    atlas        = std::make_unique<uint8_t[]>(w * h);

    atlas_w = w;
    atlas_h = h;

    auto const ok = stbtt_BakeFontBitmap(
        ttf_data,
        0,
        pixel_height,
        atlas.get(),
        atlas_w,
        atlas_h,
        32,
        96,
        cdata);

    if (!ok)
        panic("font load failed");

    // get glyhps size
    glyph_width = cdata['M' - 32].xadvance;


    stbtt_fontinfo font_info {};
    stbtt_InitFont(&font_info, ttf_data, stbtt_GetFontOffsetForIndex(ttf_data, 0));
    int ascent;
    int descent;
    int line_gap;
    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);
    auto const scale = stbtt_ScaleForPixelHeight(&font_info, pixel_height);
    baseline         = roundf((float)ascent * scale);
    glyph_height     = as<float>(ascent - descent + line_gap) * scale;
}


Quad getQuad(Font & font, uint8_t char_index, float * xpos, float * ypos)
{
    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(font.cdata, font.atlas_w, font.atlas_h, char_index, xpos, ypos, &q, 1);

    return {
        .x0 = q.x0,
        .y0 = q.y0,
        .s0 = q.s0,
        .t0 = q.t0,
        .x1 = q.x1,
        .y1 = q.y1,
        .s1 = q.s1,
        .t1 = q.t1,
    };
}
