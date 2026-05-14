#include <iostream>

#include "CliApp.h"

int main(int argc, char* argv[]) {
    CliApp app;

    if (argc == 1) {
        return app.runInteractive(std::cin, std::cout, std::cerr);
    }

    if (argc == 2) {
        return app.runBatch(argv[1], std::cout, std::cerr);
    }

    std::cerr << "Usage: ./prog [script.txt]" << '\n';
    return 1;
}
