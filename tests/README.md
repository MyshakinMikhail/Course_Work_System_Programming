# tests

Здесь лежат простые unit-тесты для `src/core`.

Тесты пока без внешнего фреймворка, на `assert`. Это удобно для первого этапа проекта и позволяет собирать их как через CMake, так и вручную.

## Запуск через CMake

Из корня проекта:

```bash
cmake --preset mingw
cmake --build --preset mingw
ctest --preset mingw --output-on-failure
```
