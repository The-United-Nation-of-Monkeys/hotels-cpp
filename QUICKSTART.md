# Быстрый старт

## Сборка проекта

```bash
cd cpp_hotels
mkdir -p build
cd build
cmake ..
make
```

## Запуск

```bash
./HotelBooking
```

Сервер запустится на `http://localhost:8080`

## Использование в CLion

1. Откройте папку `cpp_hotels` как проект в CLion
2. CLion автоматически обнаружит CMakeLists.txt
3. Нажмите Run или Shift+F10 для запуска

## Первый запуск

При первом запуске автоматически создастся база данных SQLite `hotels.db` с пустыми таблицами.

Для тестирования можно добавить данные через SQL:

```sql
INSERT INTO rooms (number, name, description, type_name, created_at, updated_at) 
VALUES ('101', 'Стандартный номер', 'Уютный номер с видом на город', 'Стандарт', datetime('now'), datetime('now'));

INSERT INTO guests (first_name, last_name, passport_number, phone, created_at, updated_at) 
VALUES ('Иван', 'Иванов', '1234567890', '+7-999-123-45-67', datetime('now'), datetime('now'));
```

## Структура URL

- `/` - Главная страница
- `/rooms/` - Список номеров
- `/rooms/<id>/` - Детали номера
- `/guests/` - Список гостей
- `/guests/create/` - Создать гостя
- `/guests/<id>/` - Детали гостя
- `/bookings/` - Список бронирований
- `/bookings/create/` - Создать бронирование
- `/bookings/<id>/` - Детали бронирования
- `/contact/` - Контакты

