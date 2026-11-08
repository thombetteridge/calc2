#include "Window.hpp"

#include <cstdint>

#include <algorithm>


#include "tom.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif


#define RGFW_IMPLEMENTATION
#define RGFW_DEBUG
#include "RGFW.h"

//--------------------------------------------------------------------------------------------------//
// Interal
//--------------------------------------------------------------------------------------------------//

namespace {


enum : uint32_t {
    MOD_KEY_SHIFT_L = 1 << 0,
    MOD_KEY_SHIFT_R = 1 << 1,
    MOD_KEY_CTRL_L  = 1 << 2,
    MOD_KEY_CTRL_R  = 1 << 3,
    MOD_KEY_ALT_L   = 1 << 4,
    MOD_KEY_ALT_R   = 1 << 5,
};

uint32_t
modifier_bit(RGFW_key key)
{
    if (key == RGFW_keyShiftL)
        return MOD_KEY_SHIFT_L;
    if (key == RGFW_keyShiftR)
        return MOD_KEY_SHIFT_R;
    if (key == RGFW_keyControlL)
        return MOD_KEY_CTRL_L;
    if (key == RGFW_keyControlR)
        return MOD_KEY_CTRL_R;
    if (key == RGFW_keyAltL)
        return MOD_KEY_ALT_L;
    if (key == RGFW_keyAltR)
        return MOD_KEY_ALT_R;
    return 0;
}


Window * window_from(RGFW_window * w)
{
    return cast<Window *>(RGFW_window_getUserPtr(w));
}

void mouseposfunc(RGFW_event const * e)
{
    Window * win = window_from(e->common.win);

    win->current_mouse.x = e->mouse.x;
    win->current_mouse.y = e->mouse.y;

    win->push_event(Window::Event {.kind = Window::Event::Kind::MouseMove});
}


void keyfunc(RGFW_keyEvent const * e)
{
    Window * win = window_from(e->win);

    RGFW_key    mapped = RGFW_physicalToMappedKey(e->value);
    Window::Key key    = as<Window::Key>(e->value);

    if (e->state) {
        win->mod_keys |= modifier_bit(mapped);
        win->key_pressed = key;
        win->push_event(
            Window::Event {
                .kind   = Window::Event::Kind::KeyDown,
                .key    = key,
                .repeat = as<bool>(e->repeat),
            });
    }

    else {
        win->mod_keys &= ~modifier_bit(mapped);
        win->key_released = key;
        win->push_event(Window::Event {.kind = Window::Event::Kind::KeyUp, .key = key});
    }
}

void keycharfunc(RGFW_event const * e)
{
    Window * win = window_from(e->common.win);

    win->key_char = as<char>(e->keyChar.value);
    win->push_event({
        .kind = Window::Event::Kind::KeyChar,
        .ch   = win->key_char,
    });
}

void mousebuttonfunc(RGFW_event const * e)
{
    Window * win = window_from(e->common.win);

    Window::Event ev = {.kind = e->button.state ? Window::Event::Kind::MouseDown : Window::Event::Kind::MouseUp};

    if (e->button.value == RGFW_mouseLeft)
        ev.button = Window::MouseButton::Left;
    else if (e->button.value == RGFW_mouseMiddle)
        ev.button = Window::MouseButton::Middle;
    else if (e->button.value == RGFW_mouseRight)
        ev.button = Window::MouseButton::Right;
    else
        return;

    win->push_event(ev);
}

void scrollfunc(RGFW_event const * e)
{
    Window * win = window_from(e->common.win);

    win->scroll += e->delta.y;
    win->push_event(
        Window::Event {
            .kind    = Window::Event::Kind::MouseWheel,
            .wheel_x = e->delta.x,
            .wheel_y = e->delta.y,
        });
}

void windowfocusfunc(RGFW_event const * e)
{
    Window * win = window_from(e->common.win);

    if (e->focus.state)
        return;
    win->mod_keys = 0;

    win->push_event(Window::Event {.kind = Window::Event::Kind::FocusLost});
}


void windowclosefunc(RGFW_event const * e)
{
    Window * win = window_from(e->common.win);

    win->running = false;
}

} // namespace


void Window::push_event(Window::Event ev)
{
    ev.x         = this->current_mouse.x;
    ev.y         = this->current_mouse.y;
    ev.mod.shift = (this->mod_keys & (MOD_KEY_SHIFT_L | MOD_KEY_SHIFT_R)) != 0;
    ev.mod.ctrl  = (this->mod_keys & (MOD_KEY_CTRL_L | MOD_KEY_CTRL_R)) != 0;
    ev.mod.alt   = (this->mod_keys & (MOD_KEY_ALT_L | MOD_KEY_ALT_R)) != 0;

    mods = ev.mod;

    this->got_input = true;


    if (ev.kind == Window::Event::Kind::MouseMove && this->event_count > 0) {
        Window::Event * const last = &this->events[(this->event_head + this->event_count - 1) % this->event_capacity];
        if (last->kind == Window::Event::Kind::MouseMove) {
            *last = ev;
            return;
        }
    }


    if (this->event_count == this->event_capacity) {
        this->event_head = (this->event_head + 1) % this->event_capacity;
        this->event_count -= 1;
    }

    this->events[(this->event_head + this->event_count) % this->event_capacity] = ev;
    this->event_count += 1;
}


//--------------------------------------------------------------------------------------------------//
// External
//--------------------------------------------------------------------------------------------------//

void rgfw_init(char const * s, uint8_t f) { RGFW_init(s, f); }
void rgfw_deinit() { RGFW_deinit(); }

Window::Window(char const * title, int w, int h)
{
    width  = w;
    height = h;

    target = std::make_unique<uint32_t[]>(width * height);

    rgfw_window = RGFW_createWindow(title, 100, 100, width, height, RGFW_windowNoResize | RGFW_windowCenter | RGFW_windowFloating);

    if (not rgfw_window) {
        panic("rgfw_window Failed");
    }


    RGFW_window_setExitKey(rgfw_window, RGFW_keyEscape);

    RGFW_window_setUserPtr(rgfw_window, this);
    RGFW_setEventCallback(RGFW_windowClose, windowclosefunc);
    RGFW_setEventCallback(RGFW_mouseMotion, mouseposfunc);
    RGFW_setEventCallback(RGFW_mouseScroll, scrollfunc);
    RGFW_setEventCallback(RGFW_windowFocusIn, windowfocusfunc);
    RGFW_setEventCallback(RGFW_windowFocusOut, windowfocusfunc);
    RGFW_setEventCallback(RGFW_keyPressed, (RGFW_genericFunc)keyfunc);
    RGFW_setEventCallback(RGFW_keyReleased, (RGFW_genericFunc)keyfunc);
    RGFW_setEventCallback(RGFW_keyChar, keycharfunc);
    RGFW_setEventCallback(RGFW_mouseButtonPressed, mousebuttonfunc);
    RGFW_setEventCallback(RGFW_mouseButtonReleased, mousebuttonfunc);

    current_mouse = mouse_pos();
    prev_mouse    = current_mouse;

    rgfw_surface = RGFW_createSurface(cast<uint8_t *>(target.get()), width, height, RGFW_formatRGBA8);

    if (not rgfw_surface) {
        panic("rgfw_surface Failed");
    }

    valid = true;

    running = true;
}

Window::~Window()
{
    if (rgfw_window) {
        RGFW_window_setUserPtr(rgfw_window, nullptr);
    }

    if (rgfw_surface) {
        RGFW_surface_free(rgfw_surface);
        rgfw_surface = nullptr;
    }

    if (rgfw_window) {
        RGFW_window_close(rgfw_window);
        rgfw_window = nullptr;
    }
}

void Window::request_close()
{
    running = false;
}

bool Window::is_running()
{
    if (!rgfw_window)
        return false;
    if (RGFW_window_shouldClose(rgfw_window))
        return false;
    return running;
}

Window::Key Window::get_pressed_key()
{
    return key_pressed;
}

Window::Key Window::get_released_key()
{
    return key_released;
}

char Window::get_last_key_char()
{
    return key_char;
}

Window::Point Window::mouse_pos()
{
    int x = 0, y = 0;
    if (rgfw_window)
        RGFW_window_getMouse(rgfw_window, &x, &y);
    return {x, y};
}

void Window::wait_for_event(int ms)
{
    RGFW_waitForEvent(ms);
}

bool Window::poll()
{
    bool       any = false;
    RGFW_event event;

    prev_mouse   = current_mouse;
    key_pressed  = Window::Key::Nil;
    key_released = Window::Key::Nil;
    got_input    = false;

    while (RGFW_window_checkEvent(rgfw_window, &event)) {
        any = true;
    }


    return any || got_input;
}

void Window::present()
{
    if (rgfw_window && rgfw_surface && valid) {
        RGFW_window_blitSurface(rgfw_window, rgfw_surface);
    }
}


void Window::render(uint32_t const * src, int w, int h, int x, int y)
{
    int const x0 = std::max(x, 0);
    int const y0 = std::max(y, 0);
    int const x1 = std::min(x + w, width);
    int const y1 = std::min(y + h, height);

    if (x0 >= x1 || y0 >= y1)
        return;

    auto const row_len = as<size_t>(x1 - x0);

    uint32_t const * s = src + (y0 - y) * w + (x0 - x);
    uint32_t *       d = target.get() + y0 * width + x0;

    for (int row = y0;
         row < y1;
         row += 1, s += w, d += width) {
        std::copy_n(s, row_len, d);
    }
}


int Window::window_width()
{
    return width;
}

int Window::window_height()
{
    return height;
}

bool Window::is_window_valid()
{
    return valid;
}


Window::Point Window::mouse_delta()
{
    Point delta = {
        current_mouse.x - prev_mouse.x,
        current_mouse.y - prev_mouse.y,
    };
    return delta;
}

bool Window::is_mouse_down(MouseButton button)
{
    if (!rgfw_window)
        return false;
    RGFW_mouseButton b;
    switch (button) {
    case Window::MouseButton::Left:
        b = RGFW_mouseLeft;
        break;
    case Window::MouseButton::Middle:
        b = RGFW_mouseMiddle;
        break;
    case Window::MouseButton::Right:
        b = RGFW_mouseRight;
        break;
    default:
        return false;
    }
    return RGFW_isMouseDown(b);
}


bool Window::is_mouse_pressed(MouseButton button)
{
    if (!rgfw_window)
        return false;
    RGFW_mouseButton b;
    switch (button) {
    case Window::MouseButton::Left:
        b = RGFW_mouseLeft;
        break;
    case Window::MouseButton::Middle:
        b = RGFW_mouseMiddle;
        break;
    case Window::MouseButton::Right:
        b = RGFW_mouseRight;
        break;
    default:
        return false;
    }
    return RGFW_isMousePressed(b);
}

float Window::mouse_scroll()
{
    float s = scroll;
    scroll  = 0.0f;
    return s;
}


bool Window::is_key_pressed(Window::Key key)
{
    return RGFW_isKeyPressed(as<RGFW_key>(key));
}


bool Window::is_key_released(Window::Key key)
{
    return RGFW_isKeyReleased(as<RGFW_key>(key));
}


bool Window::is_key_down(Window::Key key)
{
    return RGFW_isKeyDown(as<RGFW_key>(key));
}
