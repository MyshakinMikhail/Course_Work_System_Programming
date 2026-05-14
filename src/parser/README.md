# parser

Модуль преобразует SQL-подобный текст в типизированные команды для `engine`.

## Состав

- `Token` - описание токена и перечисление `TokenType`;
- `Tokenizer` - разбиение входной строки на токены;
- `AST` - структуры команд, значений, условий и выражений;
- `Parser` - синтаксический разбор команд задания 0;
- `ParseError` - ошибка синтаксиса с позицией проблемного токена.

## Поддерживаемые команды

```sql
CREATE DATABASE app;
DROP DATABASE app;
USE app;

CREATE TABLE users (id int INDEXED, name string NOT_NULL);
DROP TABLE users;
DROP TABLE app.users;

INSERT INTO users (id, name) VALUE (1, "Ann"), (2, "Bob");

SELECT * FROM users;
SELECT (id, name AS username) FROM users;
SELECT * FROM users WHERE id == 1;

UPDATE users SET name = "Bob" WHERE id == 1;
DELETE FROM users WHERE id == 1;
```

## Условия

Поддерживаются условия задания 0:

- сравнения `=`, `==`, `!=`, `<`, `>`, `<=`, `>=`;
- `BETWEEN`, интервал трактуется как `[lower, upper)`;
- `LIKE` с регулярным выражением.


## Границы ответственности

`parser` проверяет только синтаксис. Он не проверяет существование баз данных, таблиц, колонок и не работает с файловой системой. Эти проверки выполняет `engine`.
