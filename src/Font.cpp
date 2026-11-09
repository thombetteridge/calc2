#include "Font.hpp"

#include "tom.hpp"
#include <cstdint>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "fira_font.h"
auto const ttf_data      = fira_mono_ttf_compressed_data;
auto const ttf_data_size = fira_mono_ttf_compressed_size;


static unsigned int stb_decompress_length(unsigned char const * input);
static unsigned int stb_decompress(unsigned char * output, unsigned char const * i, unsigned int length);

Font::Font(float pixel_height)
{

    auto const w = 512;
    auto const h = 512;
    atlas        = std::make_unique<uint8_t[]>(w * h);

    atlas_w = w;
    atlas_h = h;

    auto ttf_buffer = std::make_unique<uint8_t[]>(stb_decompress_length(ttf_data) + 1); // extra byte for sentinel valuie

    // u32 const decompressed_size = stb_decompress_length(data);
    // U8 *      ttf_buffer        = ALLOC(allocator, U8, decompressed_size);

    if (not stb_decompress(ttf_buffer.get(), ttf_data, ttf_data_size))
        panic {};


    int ok = stbtt_BakeFontBitmap(ttf_buffer.get(), 0, pixel_height,
                                  atlas.get(), atlas_w, atlas_h,
                                  32, 96, cdata);

    if (not ok)
        panic {};

    stbtt_fontinfo info;
    if (stbtt_InitFont(&info, ttf_buffer.get(), 0)) {
        int ascent, descent, line_gap;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
        auto const scale = stbtt_ScaleForPixelHeight(&info, pixel_height);
        baseline         = roundf((float)ascent * scale);
        glyph_height     = as<float>(ascent - descent + line_gap) * scale;
    }
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

// --- stolen stb_decompress (public domain, Sean Barrett, via github.com/nothings/stb) ---

static unsigned int stb_decompress_length(unsigned char const * input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *       stb__barrier_out_e, *stb__barrier_out_b;
static unsigned char const * stb__barrier_in_b;
static unsigned char *       stb__dout;

static void stb__match(unsigned char const * data, unsigned int length)
{
    if (stb__dout + length > stb__barrier_out_e) {
        stb__dout += length;
        return;
    }
    if (data < stb__barrier_out_b) {
        stb__dout = stb__barrier_out_e + 1;
        return;
    }
    while (length--)
        *stb__dout++ = *data++;
}

static void stb__lit(unsigned char const * data, unsigned int length)
{
    if (stb__dout + length > stb__barrier_out_e) {
        stb__dout += length;
        return;
    }
    if (data < stb__barrier_in_b) {
        stb__dout = stb__barrier_out_e + 1;
        return;
    }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x) ((i[x] << 8) + i[(x) + 1])
#define stb__in3(x) ((i[x] << 16) + stb__in2((x) + 1))
#define stb__in4(x) ((i[x] << 24) + stb__in3((x) + 1))

static unsigned char const * stb_decompress_token(unsigned char const * i)
{
    if (*i >= 0x20) {
        if (*i >= 0x80)
            stb__match(stb__dout - i[1] - 1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)
            stb__match(stb__dout - (stb__in2(0) - 0x4000 + 1), i[2] + 1), i += 3;
        else
            stb__lit(i + 1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    }
    else {
        if (*i >= 0x18)
            stb__match(stb__dout - (stb__in3(0) - 0x180000 + 1), i[3] + 1), i += 4;
        else if (*i >= 0x10)
            stb__match(stb__dout - (stb__in3(0) - 0x100000 + 1), stb__in2(3) + 1), i += 5;
        else if (*i >= 0x08)
            stb__lit(i + 2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)
            stb__lit(i + 3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)
            stb__match(stb__dout - (stb__in3(1) + 1), i[4] + 1), i += 5;
        else if (*i == 0x04)
            stb__match(stb__dout - (stb__in3(1) + 1), stb__in2(4) + 1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char * buffer, unsigned int buflen)
{
    unsigned long const ADLER_MOD = 65521;
    unsigned long       s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long       blocklen = buflen % 5552;
    unsigned long       i;

    while (buflen) {
        for (i = 0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;
            buffer += 8;
        }
        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;
        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= (unsigned int)blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char * output, unsigned char const * i, unsigned int length)
{
    (void)length;
    if (stb__in4(0) != 0x57bC0000)
        return 0;
    if (stb__in4(4) != 0)
        return 0;

    unsigned int const olen = stb_decompress_length(i);
    stb__barrier_in_b       = i;
    stb__barrier_out_e      = output + olen;
    stb__barrier_out_b      = output;
    i += 16;

    stb__dout = output;
    for (;;) {
        unsigned char const * old_i = i;
        i                           = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                if (stb__dout != output + olen)
                    return 0;
                if (stb_adler32(1, output, olen) != (unsigned int)stb__in4(2))
                    return 0;
                return olen;
            }
            return 0;
        }
        if (stb__dout > output + olen)
            return 0;
    }
}
