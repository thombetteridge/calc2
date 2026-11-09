#pragma once

//--------------------------------------------------------------------------------------------------//

#include <cstdint>
#include <cstdlib>

#include <array>
#include <memory>

//--------------------------------------------------------------------------------------------------//
// rfgw fwd decl

struct RGFW_window;
struct RGFW_surface;
union RGFW_event;
struct RGFW_keyEvent;

void rgfw_init(char const * s, uint8_t f);
void rgfw_deinit();

//--------------------------------------------------------------------------------------------------//

struct Window {
    //--------------------------------------------------------------------------------------------------//
    // //
    //--------------------------------------------------------------------------------------------------//

    //--------------------------------------------------------------------------------------------------//
    // Types
    //--------------------------------------------------------------------------------------------------//

    enum struct Key : uint8_t {
        Nil           = 0,
        Escape        = '\033',
        Backtick      = '`',
        Num_0         = '0',
        Num_1         = '1',
        Num_2         = '2',
        Num_3         = '3',
        Num_4         = '4',
        Num_5         = '5',
        Num_6         = '6',
        Num_7         = '7',
        Num_8         = '8',
        Num_9         = '9',
        Minus         = '-',
        Equals        = '=',
        Back_Space    = '\b',
        Tab           = '\t',
        Space         = ' ',
        A             = 'a',
        B             = 'b',
        C             = 'c',
        D             = 'd',
        E             = 'e',
        F             = 'f',
        G             = 'g',
        H             = 'h',
        I             = 'i',
        J             = 'j',
        K             = 'k',
        L             = 'l',
        M             = 'm',
        N             = 'n',
        O             = 'o',
        P             = 'p',
        Q             = 'q',
        R             = 'r',
        S             = 's',
        T             = 't',
        U             = 'u',
        V             = 'v',
        W             = 'w',
        X             = 'x',
        Y             = 'y',
        Z             = 'z',
        Period        = '.',
        Comma         = ',',
        Slash         = '/',
        Bracket       = '[',
        Close_Bracket = ']',
        Semicolon     = ';',
        Apostrophe    = '\'',
        Back_Slash    = '\\',
        Return        = '\n',
        Enter         = Return,
        Delete        = '\177', /* 127 */
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        F25,
        Caps_Lock,
        Shift_L,
        Control_L,
        Alt_L,
        Super_L,
        Shift_R,
        Control_R,
        Alt_R,
        Super_R,
        Up,
        Down,
        Left,
        Right,
        Insert,
        Menu,
        End,
        Home,
        Page_Up,
        Page_Down,
        Num_Lock,
        Kp_Slash,
        Kp_Multiply,
        Kp_Plus,
        Kp_Minus,
        Kp_Equal,
        Kp1,
        Kp2,
        Kp3,
        Kp4,
        Kp5,
        Kp6,
        Kp7,
        Kp8,
        Kp9,
        Kp0,
        Kp_Period,
        Kp_Return,
        Scroll_Lock,
        Print_Screen,
        Pause,
        World1,
        World2,
        // Key_Last = 256 /* padding for alignment ~(175 by default) */
    };

    enum struct MouseButton : uint8_t {
        Left   = 0,
        Middle = 1,
        Right  = 2,
    };

    struct KeyMod {
        bool shift {};
        bool ctrl {};
        bool alt {};
    };


    struct Event {
        enum struct Kind : uint8_t {
            None = 0,
            KeyDown,
            KeyUp,
            KeyChar,
            MouseDown,
            MouseUp,
            MouseMove,
            MouseWheel,
            FocusLost,
        };

        Kind        kind {};
        KeyMod      mod {};
        int         x {}, y {};
        Key         key {};
        char        ch {};
        bool        repeat {};
        MouseButton button {};
        float       wheel_x {};
        float       wheel_y {};
    };


    struct Point {
        int x {}, y {};
    };


    //--------------------------------------------------------------------------------------------------//
    // Constants
    //
    //--------------------------------------------------------------------------------------------------//
    static constexpr size_t event_capacity = 256;

    //--------------------------------------------------------------------------------------------------//
    // Members
    //--------------------------------------------------------------------------------------------------//

    RGFW_window *  rgfw_window {};
    RGFW_surface * rgfw_surface {};

    std::unique_ptr<uint32_t[]> target {};

    int width {};
    int height {};

    Point  prev_mouse {};
    Point  current_mouse {};
    float  scroll {};
    Key    key_pressed {};
    Key    key_released {};
    char   key_char {};
    bool   got_input {};
    KeyMod mods {};

    uint32_t mod_keys {};
    uint32_t event_head {};
    uint32_t event_count {};

    bool valid {};
    bool running {};

    std::array<Event, event_capacity> events {};

    //--------------------------------------------------------------------------------------------------//
    // RAII
    //--------------------------------------------------------------------------------------------------//

    Window(Window const &)           = delete;
    Window operator=(Window const &) = delete;
    Window(Window *&)                = delete;
    Window operator=(Window &&)      = delete;

    Window(char const * title, int w, int h);
    ~Window();

    //--------------------------------------------------------------------------------------------------//
    // Functions
    //--------------------------------------------------------------------------------------------------//


    void request_close();

    bool is_running();
    bool poll();
    bool next_event(Event * ev);
    void render(uint32_t const * buffer, int w, int h, int x, int y);
    void present();

    void wait_for_event(int ms);

    // Queries
    int   window_width();
    int   window_height();
    bool  is_window_valid(); // false when minimized
    Point mouse_pos();
    Point mouse_delta();
    bool  is_mouse_down(MouseButton button);
    bool  is_mouse_pressed(MouseButton button);
    bool  is_key_pressed(Key key);
    bool  is_key_released(Key key);
    bool  is_key_down(Key key);
    Key   get_pressed_key();
    Key   get_released_key();
    char  get_last_key_char();
    float mouse_scroll();

    // Lengths never include a null terminator, and `text` does not need one.
    // TODO
    static char const * read_clipboard(size_t * length);
    static void         write_clipboard(char const * text, size_t length);

    //--------------------------------------------------------------------------------------------------//
    // Internal Functions
    //--------------------------------------------------------------------------------------------------//

    void push_event(Window::Event ev);
};
