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

## Текущее состояние
- UPDATE пока заглушка;
- DELETE пока заглушка;
- NOT_NULL парсится, но не применяется, потому что в core/storage нет хранения nullable/not-null;
- NULL парсится, но storage сейчас его не поддерживает;
- INDEXED сохраняется в схеме, но уникальность и использование индекса зависят от index/core;
- INSERT сейчас требует значения для всех колонок, потому что нет полноценного NULL;
- оптимизация через индекс в SELECT WHERE пока не сделана;
- UPDATE/DELETE физически не реализованы, потому что нижний storage сейчас append-only.