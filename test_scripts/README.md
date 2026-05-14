# test_scripts

Готовые batch-скрипты для ручной проверки CLI.

Запуск из корня проекта:

```bash
build/src/cli/prog test_scripts/test_script1.txt
build/src/cli/prog test_scripts/test_script2_select_where_comparisons.txt
build/src/cli/prog test_scripts/test_script3_between_like.txt
build/src/cli/prog test_scripts/test_script4_multiline_commands.txt
build/src/cli/prog test_scripts/test_script5_qualified_names.txt
build/src/cli/prog test_scripts/test_script6_error_handling.txt
build/src/cli/prog test_scripts/test_script7_current_limitations.txt
```

Перед повторным запуском одного и того же скрипта лучше очистить данные:

```bash
rm -rf data
```

`test_script6_error_handling.txt` специально содержит ошибки и проверяет, что CLI продолжает выполнять следующие команды.

`test_script7_current_limitations.txt` показывает текущие ограничения: `UPDATE`, `DELETE`, неполный `INSERT` и `NULL`.
