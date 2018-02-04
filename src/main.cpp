
#include "Window.hpp"

int main() {
    Window window;
    window.init("Clouds");

    while (!window.shouldClose()) {
        window.update();
    }
}
