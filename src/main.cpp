#include "UI.hpp"
#include <iostream>
#include <exception>

int main() {
    try {
        UI app;
        app.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal Exception: " << ex.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred." << std::endl;
        return 1;
    }
    return 0;
}
