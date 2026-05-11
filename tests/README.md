# tests

Здесь лежат простые unit-тесты для `src/core`.

Тесты пока без внешнего фреймворка, на `assert`. Это удобно для первого этапа проекта и позволяет собирать их как через CMake, так и вручную.

## Запуск через CMake

Из корня проекта:

```bash
для windows: cmake -S . -B build -G "MinGW Makefiles" 
для linux: cmake -S . -B build 

cmake --build build
cd build
ctest --output-on-failure
```
