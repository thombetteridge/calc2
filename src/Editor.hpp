#pragma once

#include <cassert>
#include <vector>

#include "Canvas.hpp"
#include "Font.hpp"
#include "tom.hpp"

#include "Window.hpp"

struct Editor {
    struct Line {
        size_t begin;
        size_t end;
    };

    std::vector<char> chars;
    std::vector<Line> lines;

    size_t cursor;
    size_t anchor;

    bool text_changed;
    bool dirty;

    bool edit_key(Window::Key key, char c, Window::KeyMod mod);
    void render(Canvas & cvs, Font const & font);

    void insert_text(char const * text, size_t len);
    void insert_char(char c);
    void replace(size_t begin, size_t end, char const * text, size_t text_len);
    void compute_lines();
    // void set_desired_col();
};

#include <algorithm>


bool Editor::edit_key(Window::Key key, char c, Window::KeyMod mod)
{
    unused(mod);

    if (key == Window::Key::Enter or key == Window::Key::Return) {
        insert_char('\n');
        return true;
    }

    if (as<int>(key) >= 32 && as<int>(key) < 127) {
        insert_char(c);
    }

    return true;
}

void Editor::compute_lines()
{
    lines.clear();

    size_t begin = 0;
    size_t end   = begin;

    for (size_t i = 0; i < chars.size(); i += 1) {
        if (chars[i] == '\n') {
            end = i;
            lines.push_back({.begin = begin, .end = end});
            begin = i + 1;
        }
    }
    lines.push_back({.begin = begin, .end = chars.size()});
}

void Editor::insert_char(char c)
{
    chars.insert(chars.begin() + cursor, c);
    cursor += 1;
    compute_lines();
}

void Editor::insert_text(char const * text, size_t len)
{
    let begin = std::min(anchor, cursor);
    let end   = std::max(anchor, cursor);
    let kept  = chars.size() - (end - begin);

    replace(begin, end, text, len);

    cursor = begin + (chars.size() - kept);
    anchor = cursor;
    // set_desired_col();
}


void Editor::replace(size_t begin, size_t end, char const * text, size_t text_len)
{
    assert(begin <= end);
    assert(end <= chars.size());

    if (begin == end && text_len == 0)
        return;

    chars.erase(chars.begin() + begin, chars.begin() + end);
    chars.insert(chars.begin() + begin, text, text + text_len);
    std::erase(chars, '\r');

    compute_lines();

    text_changed = true;
    dirty        = true;
}


void Editor::render(Canvas & cvs, Font const & font)
{
    for (size_t i = 0; i < lines.size(); i += 1) {
        let len = as<int>(lines[i].end - lines[i].begin);
        cvs.draw_text(font, chars.data() + lines[i].begin, len, 10, as<int>(as<float>(i) * (font.glyph_height + 1.f)), colour_black);
    }
}