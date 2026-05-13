#include <iostream>

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "Заглушка: интерактивный режим" << '\n';
        return 0;
    }

    if (argc == 2) {
        std::cout << "Заглушка: пакетный режим" << argv[1] << '\n';
        return 0;
    }

    std::cerr << "Usage: ./prog [script.txt]" << '\n';
    return 1;
}
