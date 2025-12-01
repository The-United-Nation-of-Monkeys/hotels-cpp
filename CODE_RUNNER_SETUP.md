# Настройка Code Runner

Если вы используете расширение Code Runner в VS Code, выполните следующие шаги:

## Шаг 1: Загрузите httplib.h

Запустите скрипт для загрузки зависимостей:

```bash
./compile_direct.sh
```

Или вручную:

```bash
mkdir -p deps
curl -L -o deps/httplib.h https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
```

## Шаг 2: Настройка Code Runner

Файл `.vscode/settings.json` уже настроен для работы с проектом. 

Если Code Runner все еще не работает, добавьте в настройки VS Code:

```json
{
    "code-runner.executorMap": {
        "cpp": "cd $dir && g++ -std=c++17 -I./include -I./deps -lsqlite3 -pthread $fileName -o $fileNameWithoutExt && $dir$fileNameWithoutExt"
    }
}
```

## Альтернатива: Используйте CMake

Рекомендуется использовать CMake для сборки:

```bash
./build.sh
```

Или вручную:

```bash
mkdir build && cd build
cmake ..
make
./HotelBooking
```

## Примечание

Code Runner компилирует отдельные файлы, но этот проект состоит из нескольких файлов. 
Для правильной работы лучше использовать CMake или скрипт `compile_direct.sh` для компиляции всего проекта.

