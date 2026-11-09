
#include "Canvas.hpp"
#include "Editor.hpp"
#include "Font.hpp"
#include "Window.hpp"

#include "tom.hpp"

int main()
{

    trace {"Test"};

    Window w {"cal", 512, 512};

    Editor ed {};
    ed.compute_lines();

    Font font {20};

    Canvas canvas(w.window_width(), w.window_height());

    canvas.clear(colour_white);

    while (w.is_running()) {
        w.wait_for_event(1000);

        if (not w.poll())
            continue;

        Window::Event ev;
        while (w.next_event(&ev)) {
            ed.handle_event(&ev);
        }

        canvas.clear(colour_white);

        ed.render(canvas, font, 3, 3, canvas.width - 3, canvas.height - 3);

        w.render(canvas.buffer.get(), canvas.width, canvas.height, 0, 0);

        w.present();
    };
}