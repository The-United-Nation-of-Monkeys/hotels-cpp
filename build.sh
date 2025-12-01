#!/bin/bash

# Скрипт для сборки проекта

echo "Сборка проекта HotelBooking..."

# Создаем директорию build если её нет
mkdir -p build
cd build

# Запускаем CMake
echo "Запуск CMake..."
cmake ..

# Компилируем
echo "Компиляция..."
make

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Сборка успешна!"
    echo "Запустите: ./build/HotelBooking"
else
    echo ""
    echo "✗ Ошибка при сборке"
    exit 1
fi

