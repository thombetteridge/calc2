
#include <string_view>

#include "Canvas.hpp"
#include "Editor.hpp"
#include "Font.hpp"
#include "Window.hpp"

#include "tom.hpp"

int main()
{
    rgfw_init("cal", 0);
    cleanup(rgfw_deinit());

    Window w {"cal", 512, 512};
    Editor ed {};

    Font font {20};

    Canvas canvas(w.window_width(), w.window_height());
    canvas.clear(colour_white);

    // constexpr std::string_view hello = "Hello World";
    // canvas.draw_text(font, hello.data(), hello.size(), 10, 200, colour_black);

    while (w.is_running()) {
        w.wait_for_event(1000);
        w.poll();

        auto const key = w.get_pressed_key();

        if (key != Window::Key::Nil) {
            ed.edit_key(key, {});
        }

        canvas.clear(colour_white);

        ed.render(canvas, font);

        w.render(canvas.buffer.data(), canvas.width, canvas.height, 0, 0);

        w.present();
    };
}