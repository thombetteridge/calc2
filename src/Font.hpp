#pragma once

#include <cstdint>
#include <memory>

#include "stb_truetype.h"

struct Font {
    std::unique_ptr<uint8_t[]> atlas;
    int                        atlas_w;
    int                        atlas_h;
    stbtt_bakedchar            cdata[96];
    float                      glyph_width;
    float                      glyph_height;
    float                      baseline;

    Font(float height);
};


struct Quad {
    float x0;
    float y0;
    float s0;
    float t0; // top-left
    float x1;
    float y1;
    float s1;
    float t1; // bottom-right
};

Quad getQuad(Font & font, uint8_t char_index, float * xpos, float * ypos);
