#pragma once

#include <vector>

#include "Canvas.hpp"
#include "Font.hpp"
#include "Window.hpp"
#include "tom.hpp"

//--------------------------------------------------------------------------------------------------//

struct Editor {

    //--------------------------------------------------------------------------------------------------//

    struct Line {
        size_t begin;
        size_t end;
    };

    struct Rect {
        int x, y, w, h;
    };

    struct Span {
        enum struct Style : u32 {
            Text     = 1 << 0,
            Squiggle = 1 << 1,
        };

        size_t         begin, end;
        Canvas::Colour colour;
        Style          style;
    };

    enum struct Flags : u32 {
        None     = 0,
        NoCursor = 1 << 0, // don't draw the cursor
        NoLine   = 1 << 1, // don't highlight the cursor's line
        ReadOnly = 1 << 2, // keyboard can move, select and copy but not change the text

        LineNumbers         = 1 << 3, // number the lines in a gutter, 1 at the top
        LineNumbersReversed = 1 << 4, // number the lines in a gutter, 1 at the bottom (stack depth)
    };

    //--------------------------------------------------------------------------------------------------//

    std::vector<char> chars;
    std::vector<Line> lines;

    size_t cursor;
    size_t anchor;
    size_t desired_column;

    size_t scroll_offset_x;
    size_t scroll_offset_y;
    size_t reveal_margin;

    Flags flags;

    int view_x, view_y, view_w, view_h;

    float wheel_rows;
    float wheel_cols;

    bool mouse_selecting;
    bool reveal_cursor;
    bool text_changed;
    bool dirty;

    //--------------------------------------------------------------------------------------------------//

    void render(Canvas & cvs, Font const & font, int x, int y, int w, int h);
    bool handle_event(Window::Event const * ev);

    bool   edit_key(Window::Key key, char c, Window::KeyMod mod);
    void   insert_text(char const * text, size_t len);
    void   insert_char(char c);
    void   insert_tab();
    void   replace(size_t begin, size_t end, char const * text, size_t text_len);
    void   compute_lines();
    size_t line_at(size_t pos);
    size_t current_line();
    size_t current_column(size_t current_line);
    void   set_desired_column();
    bool   is_selection();
    void   select_all();
    void   delete_selection();
    void   delete_line(size_t line);
    void   delete_forward();
    bool   mouse_down(int x, int y, Window::KeyMod mod);
    void   mouse_move(int x, int y);
    void   mouse_up();
    bool   mouse_wheel(int x, int y, float wheel_x, float wheel_y, Window::KeyMod mod);
    void   mouse_click(int x, int y);
    void   place_cursor(size_t pos, bool selecting);
    void   move_char_left(bool selecting);
    void   move_char_right(bool selecting);
    void   move_word_left(bool selecting);
    void   move_word_right(bool selecting);
    void   move_lines(size_t count, bool down, bool selecting);
    void   move_line_start(bool selecting);
    void   move_line_end(bool selecting);
    void   move_text_start(bool selecting);
    void   move_text_end(bool selecting);
    void   clear();
    void   select_word();
    void   reveal_cursor_fn(size_t margin);
    void   clipboard_paste();
    void   clipboard_copy();
    void   clipboard_cut();
    size_t visible_rows();
    void   backspace();
    void   delete_range(size_t begin, size_t end);
    int    text_left();
    size_t visible_cols();
    size_t longest_line();
    size_t scroll_to_show(size_t offset, size_t pos, size_t visible, size_t margin);
    void   update_scroll();
    bool   point_in_view(int x, int y);
    bool   has_gutter();
    int    gutter_cols();
    size_t pos_from_point(int x, int y);
};
//--------------------------------------------------------------------------------------------------//
