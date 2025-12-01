#!/bin/bash

# Прямая компиляция с загрузкой httplib.h
# Используйте этот скрипт если CMake не работает

echo "Загрузка cpp-httplib..."

# Создаем директорию для зависимостей
mkdir -p deps

# Скачиваем httplib.h если его нет
if [ ! -f "deps/httplib.h" ]; then
    echo "Скачивание httplib.h..."
    curl -L -o deps/httplib.h https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
    if [ $? -ne 0 ]; then
        echo "Ошибка при загрузке httplib.h"
        exit 1
    fi
fi

# Компилируем
echo "Компиляция..."
g++ -std=c++17 -I./deps -I./include src/main.cpp -lsqlite3 -pthread -o HotelBooking

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Компиляция успешна!"
    echo "Запустите: ./HotelBooking"
else
    echo ""
    echo "✗ Ошибка при компиляции"
    exit 1
fi

