
#include "Editor.hpp"

//--------------------------------------------------------------------------------------------------//

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>


static constexpr auto GLYPH_HEIGHT = 21.0f;
static constexpr auto GLYPH_WIDTH  = GLYPH_HEIGHT * (7.0f / 13.0f);

static constexpr auto OFFSET_X = (GLYPH_WIDTH / 2.0f);
static constexpr auto OFFSET_Y = (GLYPH_HEIGHT / 2.0f);

static constexpr auto CELL_W = (GLYPH_WIDTH);
static constexpr auto CELL_H = (GLYPH_HEIGHT);
static constexpr auto PAD_X  = (OFFSET_X);
static constexpr auto PAD_Y  = (OFFSET_Y);

static constexpr auto TAB_SIZE      = 4;
static constexpr auto SCROLL_MARGIN = 3;
static constexpr auto WHEEL_ROWS    = 3;
static constexpr auto WHEEL_COLS    = 3;

static constexpr auto GUTTER_MIN_DIGITS = 2;

// static constexpr auto NOTE_GAP  = 4;
// static constexpr auto NOTE_SNAP = 4;

static constexpr auto GUTTER_COLOUR = rgb(255, 255, 255);
static constexpr auto CURSOR_COLOUR = rgb(0, 255, 0);

#define VALIDATE_CURSOR()               \
    do {                                \
        /*assert(cursor >= 0); */       \
        assert(cursor <= chars.size()); \
        assert(anchor <= chars.size()); \
    } while (0)


static size_t offset_by(size_t base, int delta, size_t limit)
{
    if (delta < 0) {
        size_t const back = as<size_t>(-delta);
        base              = base > back ? base - back : 0;
    }
    else {
        base += (size_t)delta;
    }
    return std::min(base, limit);
}


static void fill_clipped(Canvas & sur, Editor::Rect clip, int x, int y, int w, int h, Canvas::Colour colour)
{
    int const x0 = std::max(x, clip.x);
    int const y0 = std::max(y, clip.y);
    int const x1 = std::min(x + w, clip.x + clip.w);
    int const y1 = std::min(y + h, clip.y + clip.h);

    if (x1 > x0 && y1 > y0)
        sur.draw_rect(x0, y0, x1 - x0, y1 - y0, colour);
}


bool Editor::edit_key(Window::Key key, char ch, Window::KeyMod mod)
{
    bool const read_only = (as<u32>(flags) & as<u32>(Flags::ReadOnly)) != 0;
    bool const shortcut  = mod.ctrl && !mod.alt;

    if (shortcut && key == Window::Key::C) {
        clipboard_copy(); // changes nothing on screen
        return true;
    }

    bool handled = true;

    if (shortcut && key == Window::Key::A) {
        select_all();
    }
    else if (shortcut && key == Window::Key::X) {
        if (read_only)
            clipboard_copy();
        else
            clipboard_cut();
    }
    else if (shortcut && key == Window::Key::V) {
        if (!read_only)
            clipboard_paste();
    }

    else if (shortcut && key == Window::Key::D) {
        select_word();
    }

    else if (key == Window::Key::Left) {
        if (mod.ctrl)
            move_word_left(mod.shift);
        else
            move_char_left(mod.shift);
    }
    else if (key == Window::Key::Right) {
        if (mod.ctrl)
            move_word_right(mod.shift);
        else
            move_char_right(mod.shift);
    }
    else if (key == Window::Key::Up) {
        move_lines(1, false, mod.shift);
    }
    else if (key == Window::Key::Down) {
        move_lines(1, true, mod.shift);
    }
    else if (key == Window::Key::Page_Up) {
        move_lines(visible_rows(), false, mod.shift);
    }
    else if (key == Window::Key::Page_Down) {
        move_lines(visible_rows(), true, mod.shift);
    }
    else if (key == Window::Key::Home) {
        if (mod.ctrl)
            move_text_start(mod.shift);
        else
            move_line_start(mod.shift);
    }
    else if (key == Window::Key::End) {
        if (mod.ctrl)
            move_text_end(mod.shift);
        else
            move_line_end(mod.shift);
    }
    else if (key == Window::Key::Back_Space) {
        if (!read_only)
            backspace();
    }
    else if (key == Window::Key::Delete) {
        if (!read_only)
            delete_forward();
    }
    else if (key == Window::Key::Tab) {
        if (!read_only)
            insert_tab();
    }
    else if (key == Window::Key::Return || key == Window::Key::Kp_Return) {
        if (!read_only)
            insert_char('\n');
    }
    else if (!read_only && !mod.ctrl && !mod.alt && ch >= 32 && ch < 127) {
        // ch is only set for keys that type a printable character
        insert_char(ch);
    }
    else {
        handled = false;
    }

    if (handled) {
        dirty = true;
        reveal_cursor_fn(SCROLL_MARGIN);
    }

    return handled;
}

void Editor::reveal_cursor_fn(size_t margin)
{
    reveal_cursor = true;
    reveal_margin = margin;
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

void Editor::insert_tab()
{
    std::array spaces = {' ', ' ', ' ', ' '};
    insert_text(spaces.data(), TAB_SIZE);
}

void Editor::insert_char(char c)
{
    chars.insert(chars.begin() + cursor, c);
    cursor += 1;
    anchor = cursor;
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
    VALIDATE_CURSOR();
}


void Editor::replace(size_t begin, size_t end, char const * text, size_t text_len)
{
    assert(begin <= end);
    assert(end <= chars.size());

    if (begin == end && text_len == 0)
        return;

    chars.erase(chars.begin() + begin, chars.begin() + end);
    cursor = chars.insert(chars.begin() + begin, text, text + text_len) - chars.begin();
    std::erase(chars, '\r');

    compute_lines();


    text_changed = true;
    dirty        = true;

    anchor = cursor;

    VALIDATE_CURSOR();
}

static int format_digits(char * buf, int width, size_t n)
{
    int len = 0;
    do {
        buf[width - 1 - len] = (char)('0' + n % 10);
        n /= 10;
        len += 1;
    } while (n > 0 && len < width);
    return len;
}


void Editor::render(Canvas & cvs, Font const & font, int x, int y, int w, int h)
{
    dirty = false;

    /* save location */
    view_x = x;
    view_y = y;
    view_w = w;
    view_h = h;

    update_scroll();

    Rect const clip = {x, y, w, h};

    size_t const rows = visible_rows();
    size_t const cols = visible_cols();

    size_t const start_line  = as<size_t>(scroll_offset_y);
    size_t const finish_line = std::min(start_line + rows, lines.size());

    size_t const start_col  = as<size_t>(scroll_offset_x);
    size_t const finish_col = start_col + cols + 1; // the margin leaves room for one more cell

    size_t const cursor_row = current_line();
    size_t const cursor_col = current_column(cursor_row);

    int const text_x = x + text_left();
    int const text_y = y + as<int>(PAD_Y);

    bool const cursor_row_shown = cursor_row >= start_line && cursor_row < finish_line;
    bool const cursor_col_shown = cursor_col >= start_col && cursor_col <= finish_col;

    // // draw line
    // if (cursor_row_shown && !(flags & ED_NO_LINE)) {
    //     fill_clipped(sur, clip,
    //                  x,
    //                  text_y + as<int>(cursor_row - start_line) * CELL_H,
    //                  w,
    //                  CELL_H,
    //                  LINE_COLOUR);
    // }

    // draw line numbers, right-aligned in the gutter
    if (has_gutter()) {
        int const  width    = gutter_cols() - 1;
        bool const reversed = (fromEnum(flags) & fromEnum(Flags::LineNumbersReversed)) != 0;

        for (auto line = start_line; line < finish_line; line += 1) {
            int const glyph_y = as<int>(as<float>(text_y) + as<float>(line - start_line) * CELL_H);
            if (as<float>(glyph_y) + CELL_H > as<float>(y + h))
                break;

            char         digits[24];
            size_t const number = reversed ? lines.size() - line : line + 1;
            int const    len    = format_digits(digits, width, number);
            // Colour const colour = line == cursor_row ? TEXT_COLOUR : GUTTER_COLOUR;
            Canvas::Colour const colour = GUTTER_COLOUR;

            for (int col = width - len; col < width; col += 1) {
                int const glyph_x = as<int>(as<float>(x) + PAD_X + as<float>(col) * CELL_W);
                if (as<float>(glyph_x) + CELL_W > as<float>(x + w))
                    break;
                cvs.draw_text(font, &digits[col], 1, glyph_x, glyph_y, colour);
            }
        }
    }

    // draw cursor
    if (cursor_row_shown && cursor_col_shown && !(as<u32>(flags) & as<u32>(Flags::NoCursor))) {
        fill_clipped(cvs, clip,
                     as<int>(as<float>(text_x) + as<float>(cursor_col - start_col) * CELL_W - as<float>(1)),
                     as<int>(as<float>(text_y) + as<float>(cursor_row - start_line) * CELL_H),
                     1,
                     as<int>(CELL_H),
                     CURSOR_COLOUR);
    }

    // draw selection, one rectangle per line
    if (is_selection()) {
        size_t const selection_begin = std::min(anchor, cursor);
        size_t const selection_end   = std::max(anchor, cursor);

        for (auto line = start_line; line < finish_line; line += 1) {
            size_t const line_begin = lines[line].begin;
            size_t const line_end   = lines[line].end;

            size_t const begin = std::max(selection_begin, line_begin);
            size_t const end   = std::min(selection_end, line_end);
            if (begin >= end)
                continue;

            size_t const col_begin = std::max(begin - line_begin, start_col);
            size_t const col_end   = std::min(end - line_begin, finish_col);
            if (col_begin >= col_end)
                continue;

            fill_clipped(cvs, clip,
                         as<int>(as<float>(text_x) + as<float>(col_begin - start_col) * CELL_W),
                         as<int>(as<float>(text_y) + as<float>(line - start_line) * CELL_H),
                         as<int>(as<float>(col_end - col_begin) * CELL_W),
                         as<int>(CELL_H),
                         CURSOR_COLOUR);
        }
    }

    // draw text, one glyph per cell. Anything unprintable (tabs included) is left as a blank cell.
    for (auto line = start_line; line < finish_line; line += 1) {
        size_t const line_begin = lines[line].begin;
        size_t const line_len   = lines[line].end - line_begin;
        size_t const stop       = std::min(line_len, finish_col);

        int const glyph_y = as<int>(as<float>(text_y) + as<float>(line - start_line) * CELL_H);
        if (as<float>(glyph_y) + CELL_H > as<float>(y + h))
            break;

        // LineSpans line_spans;
        // line_spans_collect(line_begin + start_col, line_begin + stop, &line_spans);

        for (size_t col = start_col; col < stop; col += 1) {
            char const * const glyph   = chars.data() + line_begin + col;
            int const          glyph_x = as<int>(as<float>(text_x) + as<float>(col - start_col) * CELL_W);

            if (as<float>(glyph_x) + CELL_W > as<float>(x + w))
                break;
            if (*glyph <= ' ' || *glyph >= 127)
                continue;

            cvs.draw_text(font, glyph, 1, glyph_x, glyph_y, rgb(0, 0, 0));
        }
    }
}

size_t Editor::line_at(size_t pos)
{
    assert(lines.size() >= 1);

    size_t lo = 0;
    size_t hi = lines.size() - 1;

    while (lo < hi) {
        size_t const mid = lo + (hi - lo + 1) / 2;
        if (lines[mid].begin <= pos)
            lo = mid;
        else
            hi = mid - 1;
    }
    return lo;
}


size_t Editor::current_line()
{
    return line_at(cursor);
}

size_t Editor::current_column(size_t current_line)
{
    return cursor - lines[current_line].begin;
}

void Editor::set_desired_column()
{
    auto const line = current_line();
    desired_column  = current_column(line);
}

bool Editor::is_selection()
{
    return cursor != anchor;
}

void Editor::select_all()
{
    anchor = 0;
    cursor = chars.size();
    VALIDATE_CURSOR();
}

void Editor::backspace()
{
    if (is_selection()) {
        delete_selection();
    }
    else if (cursor > 0) {
        delete_range(cursor - 1, cursor);
    }
    anchor = cursor;
    VALIDATE_CURSOR();
}

void Editor::delete_forward()
{
    if (is_selection())
        delete_selection();
    else if (cursor < chars.size())
        delete_range(cursor, cursor + 1);
}


void Editor::delete_range(size_t begin, size_t end)
{
    replace(begin, end, nullptr, 0);

    cursor = begin;
    anchor = begin;
    set_desired_column();
    VALIDATE_CURSOR();
}

void Editor::delete_selection()
{
    delete_range(std::min(anchor, cursor), std::max(anchor, cursor));
}

void Editor::delete_line(size_t line)
{
    assert(line < lines.size());

    Line const line_to_delete = lines[line];
    bool const has_next       = line + 1 < lines.size();
    bool const has_prev       = line > 0;

    size_t const begin = (!has_next && has_prev) ? line_to_delete.begin - 1 : line_to_delete.begin;
    size_t const end   = has_next ? line_to_delete.end + 1 : line_to_delete.end;

    delete_range(begin, end);
    VALIDATE_CURSOR();
}


enum struct CharClass : uint8_t {
    Space,
    Word,
    Punct,
};

static CharClass char_class(char c)
{
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        return CharClass::Space;
    if (std::ispunct(as<uint8_t>(c)))
        return CharClass::Punct;
    return CharClass::Word;
}


void Editor::place_cursor(size_t pos, bool selecting)
{
    cursor = pos;

    if (!selecting)
        anchor = cursor;

    VALIDATE_CURSOR();
}


void Editor::move_char_left(bool selecting)
{
    place_cursor(cursor > 0 ? cursor - 1 : 0, selecting);
    set_desired_column();
}


void Editor::move_char_right(bool selecting)
{
    place_cursor(std::min(cursor + 1, chars.size()), selecting);
    set_desired_column();
}


void Editor::move_word_left(bool selecting)
{
    size_t pos = cursor;

    // Skip whitespace
    while (pos > 0 && char_class(chars[pos - 1]) == CharClass::Space)
        pos -= 1;

    if (pos > 0) {
        CharClass cls = char_class(chars[pos - 1]);
        while (pos > 0 && char_class(chars[pos - 1]) == cls)
            pos -= 1;
    }

    place_cursor(pos, selecting);
    set_desired_column();
}


void Editor::move_word_right(bool selecting)
{
    size_t pos = cursor;

    // Skip whitespace
    while (pos < chars.size() && char_class(chars[pos]) == CharClass::Space)
        pos += 1;

    if (pos < chars.size()) {
        CharClass cls = char_class(chars[pos]);
        while (pos < chars.size() && char_class(chars[pos]) == cls)
            pos += 1;
    }

    place_cursor(pos, selecting);
    set_desired_column();
}


void Editor::move_lines(size_t count, bool down, bool selecting)
{
    size_t const line = current_line();
    size_t       target;

    if (down)
        target = std::min(line + count, lines.size() - 1);
    else
        target = line > count ? line - count : 0;

    size_t pos = cursor;
    if (target != line) {
        Line const to = lines[target];
        pos           = std::min(to.begin + desired_column, to.end);
    }

    place_cursor(pos, selecting);
}


void Editor::move_line_start(bool selecting)
{
    place_cursor(lines[current_line()].begin, selecting);
    set_desired_column();
}


void Editor::move_line_end(bool selecting)
{
    place_cursor(lines[current_line()].end, selecting);
    set_desired_column();
}


void Editor::move_text_start(bool selecting)
{
    place_cursor(0, selecting);
    set_desired_column();
}


void Editor::move_text_end(bool selecting)
{
    place_cursor(chars.size(), selecting);
    set_desired_column();
}


size_t Editor::visible_rows()
{
    auto const rows = (as<float>(view_h) - PAD_Y) / CELL_H - 1;
    return as<size_t>(std::max(rows, 1.0f));
}


void Editor::select_word()
{
    size_t start = cursor;
    if (start > 0) {
        CharClass cls = char_class(chars[start - 1]);
        while (start > 0 && char_class(chars[start - 1]) == cls)
            start -= 1;
    }

    size_t end = cursor;

    if (end < chars.size()) {
        CharClass cls = char_class(chars[end]);
        while (end < chars.size() && char_class(chars[end]) == cls)
            end += 1;
    }

    anchor = start;
    cursor = end;
    VALIDATE_CURSOR();
}


void Editor::clipboard_paste()
{
    size_t       text_length = 0;
    char const * text        = Window::read_clipboard(&text_length);

    if (text == NULL || text_length <= 0)
        return;

    insert_text(text, text_length);
}


void Editor::clipboard_copy()
{
    size_t begin = std::min(cursor, anchor);
    size_t end   = std::max(cursor, anchor);

    if (begin == end) {
        // no selection copy whole line
        Line const cur_line = lines[current_line()];
        begin               = cur_line.begin;
        end                 = cur_line.end;
    }

    if (end > begin)
        Window::write_clipboard(&chars[begin], (end - begin));
}


void Editor::clipboard_cut()
{
    clipboard_copy();

    if (is_selection()) {
        delete_selection();
    }
    else {
        // no selection delete line
        delete_line(current_line());
    }
}


void Editor::clear()
{
    chars.clear();
    lines.clear();

    cursor          = 0;
    anchor          = 0;
    desired_column  = 0;
    scroll_offset_x = 0;
    scroll_offset_y = 0;
    reveal_cursor   = false;
    // clear_spans();
}

bool Editor::has_gutter()
{
    return (as<u32>(flags) & (as<u32>(Flags::LineNumbers) | as<u32>(Flags::LineNumbersReversed))) != 0;
}


// Columns taken by the gutter: the widest line number and one blank column before the text.
int Editor::gutter_cols()
{
    if (not has_gutter())
        return 0;

    int digits = 1;
    for (size_t n = lines.size(); n >= 10; n /= 10)
        digits += 1;

    return std::max(digits, GUTTER_MIN_DIGITS) + 1;
}


// From the view's left edge to where the first text column starts.
int Editor::text_left()
{
    return as<int>(PAD_X + as<float>(gutter_cols()) * CELL_W);
}


size_t Editor::visible_cols()
{
    int const cols = as<int>(as<float>(view_w - text_left()) / CELL_W - 1.0f);
    return as<size_t>(std::max(cols, 1));
}


size_t Editor::longest_line()
{
    size_t longest = 0;
    for (auto const line : lines) {
        longest = std::max(longest, line.end - line.begin);
    }
    return longest;
}


size_t Editor::scroll_to_show(size_t offset, size_t pos, size_t visible, size_t margin)
{
    if (pos < offset + margin)
        offset = pos > margin ? pos - margin : 0;

    if (pos + margin + 1 > offset + visible)
        offset = pos + margin + 1 - visible;

    return offset;
}


void Editor::update_scroll()
{
    size_t const rows = visible_rows();
    size_t const cols = visible_cols();

    if (reveal_cursor) {
        size_t const row = current_line();
        size_t const col = current_column(row);

        scroll_offset_y = scroll_to_show(scroll_offset_y, row, rows, std::min(reveal_margin, rows / 2));
        scroll_offset_x = scroll_to_show(scroll_offset_x, col, cols, std::min(reveal_margin, cols / 2));
        reveal_cursor   = false;
    }

    size_t const max_scroll = (lines.size() > rows) ? lines.size() - rows : 0;
    scroll_offset_y         = std::min(scroll_offset_y, max_scroll);
}


bool Editor::point_in_view(int x, int y)
{
    return x >= view_x && x < view_x + view_w && y >= view_y && y < view_y + view_h;
}


static int floor_div(int a, int b)
{
    return a >= 0 ? a / b : -((b - 1 - a) / b);
}


size_t Editor::pos_from_point(int x, int y)
{
    int const rel_x = x - view_x - text_left();
    int const rel_y = y - view_y - as<int>(PAD_Y);

    int col_offset = floor_div(as<int>(as<float>(rel_x) + CELL_W / 2.0f), as<int>(CELL_W));
    int row_offset = floor_div(rel_y, as<int>(CELL_H));

    if (point_in_view(x, y)) {
        col_offset = std::min(col_offset, as<int>(visible_cols()) + 1);
        col_offset = std::max(col_offset, 0);
        row_offset = std::min(row_offset, as<int>(visible_rows()) - 1);
        row_offset = std::max(row_offset, 0);
    }

    size_t const row  = offset_by(scroll_offset_y, row_offset, lines.size() - 1);
    Line const   line = lines[row];
    size_t const col  = offset_by(scroll_offset_x, col_offset, line.end - line.begin);

    return line.begin + col;
}


bool Editor::mouse_down(int x, int y, Window::KeyMod mod)
{
    if (not point_in_view(x, y))
        return false;

    cursor = pos_from_point(x, y);
    if (!mod.shift)
        anchor = cursor; // shift-click extends the existing selection

    mouse_selecting = true;
    dirty           = true;
    reveal_cursor   = false; // it was put somewhere visible, the view stays where it is
    set_desired_column();
    return true;
}


void Editor::mouse_move(int x, int y)
{
    if (!mouse_selecting)
        return;

    size_t const pos = pos_from_point(x, y);
    if (pos != cursor) {
        cursor = pos; // anchor stays put, so this is the selection
        dirty  = true;
        set_desired_column();

        // dragged past an edge: follow, with no margin so it moves a line at a time
        if (not point_in_view(x, y))
            reveal_cursor_fn(0);
    }
}


void Editor::mouse_up()
{
    mouse_selecting = false;
}


void Editor::mouse_click(int x, int y)
{
    if (mouse_down(x + view_x, y + view_y, {}))
        mouse_up();
}


bool Editor::mouse_wheel(int x, int y, float wheel_x, float wheel_y, Window::KeyMod mod)
{
    if (not point_in_view(x, y))
        return false;

    if (mod.shift) {
        wheel_x += wheel_y;
        wheel_y = 0.0f;
    }

    update_scroll();

    wheel_rows += wheel_y * as<float>(WHEEL_ROWS);
    wheel_cols += wheel_x * as<float>(WHEEL_COLS);

    wheel_rows = std::min(wheel_rows, 1000.0f);
    wheel_rows = std::max(wheel_rows, -1000.0f);
    wheel_cols = std::min(wheel_cols, 1000.0f);
    wheel_cols = std::max(wheel_cols, -1000.0f);

    int const rows = as<int>(wheel_rows);
    int const cols = as<int>(wheel_cols);

    wheel_rows -= as<float>(rows);
    wheel_cols -= as<float>(cols);

    size_t const old_y = scroll_offset_y;
    size_t const old_x = scroll_offset_x;

    if (rows != 0) {
        size_t const visible    = visible_rows();
        size_t const max_scroll = (lines.size() > visible) ? lines.size() - visible : 0;
        scroll_offset_y         = offset_by(old_y, -rows, max_scroll);
    }

    if (cols != 0) {
        size_t const visible    = visible_cols();
        size_t const longest    = longest_line();
        size_t const max_scroll = (longest + 1 > visible) ? longest + 1 - visible : 0;
        scroll_offset_x         = offset_by(old_x, -cols, std::max(max_scroll, old_x));
    }

    if (scroll_offset_y != old_y || scroll_offset_x != old_x)
        dirty = true;

    return true;
}


bool Editor::handle_event(Window::Event const * ev)
{

    if (ev->kind == Window::Event::Kind::KeyDown)
        return edit_key(ev->key, 0, ev->mod);

    if (ev->kind == Window::Event::Kind::KeyChar)
        return edit_key(Window::Key::Nil, ev->ch, ev->mod);

    if (ev->kind == Window::Event::Kind::MouseDown)
        return ev->button == Window::MouseButton::Left && mouse_down(ev->x, ev->y, ev->mod);

    if (ev->kind == Window::Event::Kind::MouseMove) {
        bool const selecting = mouse_selecting;
        mouse_move(ev->x, ev->y);
        return selecting;
    }

    if (ev->kind == Window::Event::Kind::MouseUp) {
        if (ev->button != Window::MouseButton::Left || !mouse_selecting)
            return false;
        mouse_up();
        return true;
    }

    if (ev->kind == Window::Event::Kind::MouseWheel)
        return mouse_wheel(ev->x, ev->y, ev->wheel_x, ev->wheel_y, ev->mod);

    if (ev->kind == Window::Event::Kind::FocusLost)
        mouse_up();

    return false;
}
