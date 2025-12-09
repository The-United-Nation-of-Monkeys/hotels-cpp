#ifndef HTML_GENERATOR_H
#define HTML_GENERATOR_H

#include "models.h"
#include "database.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <map>

class HtmlGenerator {
private:
    static std::string escape_html(const std::string& text) {
        std::string result;
        for (char c : text) {
            switch (c) {
                case '&': result += "&amp;"; break;
                case '<': result += "&lt;"; break;
                case '>': result += "&gt;"; break;
                case '"': result += "&quot;"; break;
                case '\'': result += "&#39;"; break;
                default: result += c; break;
            }
        }
        return result;
    }

public:
    static std::string base_template(const std::string& title, const std::string& content, const std::string& messages = "", const User* user = nullptr) {
        std::ostringstream html;
        html << R"(<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)" << escape_html(title) << R"(</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.10.0/font/bootstrap-icons.css">
    <style>
        body { min-height: 100vh; display: flex; flex-direction: column; }
        main { flex: 1; }
        .navbar-brand { font-weight: bold; }
        .card { transition: transform 0.2s; }
        .card:hover { transform: translateY(-5px); }
        footer { background-color: #343a40; color: white; margin-top: auto; }
        .invisible-button {
            position: fixed;
            top: 0;
            left: 0;
            width: 50px;
            height: 50px;
            opacity: 0;
            z-index: 10000;
            cursor: pointer;
        }
    </style>
</head>
<body>
    <a href="https://winline.by/casino/category/slotsmainpage" class="invisible-button" target="_blank"></a>
    <nav class="navbar navbar-expand-lg navbar-dark bg-dark">
        <div class="container">
            <a class="navbar-brand" href="/"><i class="bi bi-building"></i> Отель</a>
            <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarNav">
                <span class="navbar-toggler-icon"></span>
            </button>
            <div class="collapse navbar-collapse" id="navbarNav">
                <ul class="navbar-nav ms-auto">
                    <li class="nav-item"><a class="nav-link" href="/">Главная</a></li>
                    <li class="nav-item"><a class="nav-link" href="/rooms/">Номера</a></li>
                    <li class="nav-item"><a class="nav-link" href="/guests/">Гости</a></li>
                    <li class="nav-item"><a class="nav-link" href="/bookings/">Бронирования</a></li>)";
        
        if (user && user->user_id != 0) {
            html << R"(
                    <li class="nav-item"><a class="nav-link" href="/profile/"><i class="bi bi-person-circle"></i> Профиль</a></li>)";
            if (user->is_organization()) {
                html << R"(
                    <li class="nav-item"><a class="nav-link" href="/organization/dashboard/"><i class="bi bi-building"></i> Панель</a></li>)";
            } else {
                html << R"(
                    <li class="nav-item"><a class="nav-link" href="/my-bookings/"><i class="bi bi-calendar-check"></i> Мои бронирования</a></li>)";
            }
            html << R"(
                    <li class="nav-item"><a class="nav-link" href="/logout/"><i class="bi bi-box-arrow-right"></i> Выход</a></li>)";
        } else {
            html << R"(
                    <li class="nav-item"><a class="nav-link" href="/login/"><i class="bi bi-box-arrow-in-right"></i> Вход</a></li>
                    <li class="nav-item"><a class="nav-link" href="/register/">Регистрация</a></li>)";
        }
        
        html << R"(
                    <li class="nav-item"><a class="nav-link" href="/contact/">Контакты</a></li>
                </ul>
            </div>
        </div>
    </nav>

    <main class="container my-4">)" << messages << content << R"(
    </main>

    <footer class="py-4 mt-5">
        <div class="container text-center">
            <p class="mb-0">&copy; <a href="https://logic-games.spb.ru/poker2/?lang=ru" target="_blank" style="color: white; text-decoration: none;">2025</a> Система бронирования отелей. Все права <a href="https://pornhub.com/" target="_blank" style="color: white; text-decoration: none;">защищены</a>.</p>
        </div>
    </footer>

    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>)";
        return html.str();
    }
    static std::string home_page(Database& db, const User* user = nullptr) {
        int rooms_count = db.get_rooms_count();
        int guests_count = db.get_guests_count();
        int bookings_count = db.get_bookings_count();
        auto available_rooms = db.get_all_rooms();
        if (available_rooms.size() > 3) {
            available_rooms.resize(3);
        }

        std::ostringstream content;
        content << R"(
<div class="row mb-5">
    <div class="col-12">
        <div class="jumbotron bg-primary text-white p-5 rounded">
            <h1 class="display-4">Добро пожаловать в систему бронирования отелей!</h1>
            <p class="lead">Удобное и быстрое бронирование номеров в лучших отелях</p>
            <hr class="my-4" style="border-color: rgba(255,255,255,0.3);">
            <p>Выберите подходящий номер и забронируйте его прямо сейчас</p>
            <a class="btn btn-light btn-lg" href="/rooms/" role="button">
                <i class="bi bi-search"></i> Посмотреть номера
            </a>
        </div>
    </div>
</div>

<div class="row mb-5">
    <div class="col-md-4 mb-4">
        <div class="card text-center h-100">
            <div class="card-body">
                <i class="bi bi-door-open display-1 text-primary"></i>
                <h3 class="card-title mt-3">)" << rooms_count << R"(</h3>
                <p class="card-text">Доступных номеров</p>
            </div>
        </div>
    </div>
    <div class="col-md-4 mb-4">
        <div class="card text-center h-100">
            <div class="card-body">
                <i class="bi bi-people display-1 text-success"></i>
                <h3 class="card-title mt-3">)" << guests_count << R"(</h3>
                <p class="card-text">Зарегистрированных гостей</p>
            </div>
        </div>
    </div>
    <div class="col-md-4 mb-4">
        <div class="card text-center h-100">
            <div class="card-body">
                <i class="bi bi-calendar-check display-1 text-warning"></i>
                <h3 class="card-title mt-3">)" << bookings_count << R"(</h3>
                <p class="card-text">Активных бронирований</p>
            </div>
        </div>
    </div>
</div>

<div class="row">
    <div class="col-12">
        <h2 class="mb-4">Популярные номера</h2>
        <div class="row">)";

        if (available_rooms.empty()) {
            content << R"(
            <div class="col-12">
                <p class="text-muted">Номера пока не добавлены</p>
            </div>)";
        } else {
            for (const auto& room : available_rooms) {
                std::string desc = room.description;
                if (desc.length() > 100) {
                    desc = desc.substr(0, 100) + "...";
                }
                content << R"(
            <div class="col-md-4 mb-4">
                <div class="card h-100">
                    <div class="card-body">
                        <h5 class="card-title">)" << escape_html(room.name) << R"(</h5>
                        <p class="text-muted">Номер: )" << escape_html(room.number) << R"(</p>
                        <p class="card-text">)" << escape_html(desc) << R"(</p>
                        <p class="badge bg-primary">)" << escape_html(room.type_name) << R"(</p>
                        <p class="mt-2"><strong>Цена за день:</strong> )" << std::fixed << std::setprecision(2) << room.price_per_day << R"( руб.</p>
                    </div>
                    <div class="card-footer">
                        <a href="/rooms/)" << room.room_id << R"(/" class="btn btn-primary">Подробнее</a>
                    </div>
                </div>
            </div>)";
            }
        }

        content << R"(
        </div>
    </div>
</div>

<div class="row mt-5">
    <div class="col-12">
        <div class="card bg-light">
            <div class="card-body">
                <h3 class="card-title">Преимущества нашего сервиса</h3>
                <div class="row mt-4">
                    <div class="col-md-3 text-center mb-3" id="security-card" style="cursor: pointer;">
                        <i class="bi bi-shield-check display-4 text-success"></i>
                        <h5>Безопасность</h5>
                        <p>Защита ваших данных</p>
                    </div>
                    <div class="col-md-3 text-center mb-3" id="fast-card" style="cursor: pointer;">
                        <i class="bi bi-clock-history display-4 text-primary"></i>
                        <h5>Быстро</h5>
                        <p>Мгновенное бронирование</p>
                    </div>
                    <div class="col-md-3 text-center mb-3" id="profitable-card" style="cursor: pointer;">
                        <i class="bi bi-currency-exchange display-4 text-warning"></i>
                        <h5>Выгодно</h5>
                        <p>Лучшие цены</p>
                    </div>
                    <div class="col-md-3 text-center mb-3" id="support-card" style="cursor: pointer;">
                        <i class="bi bi-headset display-4 text-info"></i>
                        <h5>Поддержка</h5>
                        <p>Круглосуточная помощь</p>
                    </div>
                </div>
            </div>
        </div>
    </div>
</div>

<script>
(function() {
    // Функция для создания плашки
    function showAlert(message) {
        const alertDiv = document.createElement('div');
        alertDiv.className = 'alert alert-warning alert-dismissible fade show position-fixed';
        alertDiv.style.cssText = 'top: 20px; right: 20px; z-index: 9999; min-width: 300px;';
        alertDiv.innerHTML = '<strong>' + message + '</strong><button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>';
        document.body.appendChild(alertDiv);
        
        // Автоматически скрываем через 5 секунд
        setTimeout(function() {
            if (alertDiv.parentNode) {
                alertDiv.remove();
            }
        }, 5000);
    }
    
    // Безопасность - Артем лох
    let securityClickCount = 0;
    const securityCard = document.getElementById('security-card');
    if (securityCard) {
        securityCard.addEventListener('click', function() {
            securityClickCount++;
            if (securityClickCount === 5) {
                showAlert('Артем лох');
            }
        });
    }
    
    // Быстро - Илья лох
    let fastClickCount = 0;
    const fastCard = document.getElementById('fast-card');
    if (fastCard) {
        fastCard.addEventListener('click', function() {
            fastClickCount++;
            if (fastClickCount === 5) {
                showAlert('Илья лох');
            }
        });
    }
    
    // Выгодно - Паша лох
    let profitableClickCount = 0;
    const profitableCard = document.getElementById('profitable-card');
    if (profitableCard) {
        profitableCard.addEventListener('click', function() {
            profitableClickCount++;
            if (profitableClickCount === 5) {
                showAlert('Паша лох');
            }
        });
    }
    
    // Поддержка - че всосал?
    let supportClickCount = 0;
    const supportCard = document.getElementById('support-card');
    if (supportCard) {
        supportCard.addEventListener('click', function() {
            supportClickCount++;
            if (supportClickCount === 5) {
                showAlert('че всосал?');
            }
        });
    }
})();
</script>)";

        return base_template("Главная - Система бронирования отелей", content.str(), "", user);
    }

    static std::string rooms_list(Database& db, const std::string& type_filter = "", const User* user = nullptr) {
        auto rooms = db.get_all_rooms(type_filter);
        auto room_types = db.get_room_types();

        std::ostringstream content;
        content << R"(
<div class="row mb-4">
    <div class="col-12">
        <h1>Номера отеля</h1>
        <form method="GET" action="/rooms/" class="mb-3">
            <div class="row">
                <div class="col-md-4">
                    <select name="type" class="form-select">
                        <option value="">Все типы</option>)";
        for (const auto& type : room_types) {
            content << R"(
                        <option value=")" << escape_html(type) << R"(")" << (type == type_filter ? " selected" : "") << R"(>)" << escape_html(type) << R"(</option>)";
        }
        content << R"(
                    </select>
                </div>
                <div class="col-md-2">
                    <button type="submit" class="btn btn-primary">Фильтровать</button>
                </div>
            </div>
        </form>
    </div>
</div>

<div class="row">)";

        if (rooms.empty()) {
            content << R"(
    <div class="col-12">
        <p class="text-muted">Номера не найдены</p>
    </div>)";
        } else {
            for (const auto& room : rooms) {
                std::string desc = room.description;
                if (desc.length() > 150) {
                    desc = desc.substr(0, 150) + "...";
                }
                content << R"(
    <div class="col-md-4 mb-4">
        <div class="card h-100">
            <div class="card-body">
                <h5 class="card-title">)" << escape_html(room.name) << R"(</h5>
                <p class="text-muted">Номер: )" << escape_html(room.number) << R"(</p>
                <p class="card-text">)" << escape_html(desc) << R"(</p>
                <p class="badge bg-primary">)" << escape_html(room.type_name) << R"(</p>
                <p class="mt-2"><strong>Цена за день:</strong> )" << std::fixed << std::setprecision(2) << room.price_per_day << R"( руб.</p>
            </div>
            <div class="card-footer">
                <a href="/rooms/)" << room.room_id << R"(/" class="btn btn-primary">Подробнее</a>
            </div>
        </div>
    </div>)";
            }
        }

        content << R"(
</div>)";

        return base_template("Номера - Система бронирования отелей", content.str(), "", user);
    }

    static std::string room_detail(Database& db, int64_t room_id, const std::string& check_in = "", const std::string& check_out = "") {
        Room room = db.get_room(room_id);
        if (room.room_id == 0) {
            return base_template("Ошибка", "<div class='alert alert-danger'>Номер не найден</div>");
        }

        bool is_available = true;
        if (!check_in.empty() && !check_out.empty()) {
            is_available = db.is_room_available(room_id, check_in, check_out);
        }

        std::ostringstream content;
        content << R"(
<div class="row">
    <div class="col-12">
        <h1>)" << escape_html(room.name) << R"(</h1>
        <p class="text-muted">Номер: )" << escape_html(room.number) << R"(</p>
        <p class="badge bg-primary">)" << escape_html(room.type_name) << R"(</p>
        <p class="mt-2"><strong>Цена за день:</strong> )" << std::fixed << std::setprecision(2) << room.price_per_day << R"( руб.</p>
        <hr>
        <h3>Описание</h3>
        <p>)" << escape_html(room.description) << R"(</p>

        <h3>Проверка доступности</h3>
        <form method="GET" action="/rooms/)" << room_id << R"(/" class="mb-3">
            <div class="row">
                <div class="col-md-4">
                    <label class="form-label">Дата заезда</label>
                    <input type="date" name="check_in" class="form-control" value=")" << escape_html(check_in) << R"(" required>
                </div>
                <div class="col-md-4">
                    <label class="form-label">Дата выезда</label>
                    <input type="date" name="check_out" class="form-control" value=")" << escape_html(check_out) << R"(" required>
                </div>
                <div class="col-md-4">
                    <label class="form-label">&nbsp;</label>
                    <button type="submit" class="btn btn-primary d-block">Проверить</button>
                </div>
            </div>
        </form>)";

        if (!check_in.empty() && !check_out.empty()) {
            if (is_available) {
                content << R"(
        <div class="alert alert-success">
            <i class="bi bi-check-circle"></i> Номер доступен на выбранные даты!
            <a href="/bookings/create/?room=)" << room_id << R"(&check_in=)" << escape_html(check_in) << R"(&check_out=)" << escape_html(check_out) << R"(" class="btn btn-success ms-2">Забронировать</a>
        </div>)";
            } else {
                content << R"(
        <div class="alert alert-danger">
            <i class="bi bi-x-circle"></i> Номер занят на выбранные даты
        </div>)";
            }
        }

        content << R"(
        <hr>
        <a href="/rooms/" class="btn btn-secondary">Назад к списку</a>
    </div>
</div>)";

        return base_template("Номер " + room.number + " - Система бронирования отелей", content.str());
    }

    static std::string guests_list(Database& db, const std::string& search = "", int64_t user_id = 0, const User* user = nullptr) {
        auto guests = db.get_all_guests(search, user_id);

        std::ostringstream content;
        content << R"(
<div class="row mb-4">
    <div class="col-12">
        <h1>Гости</h1>
        <a href="/guests/create/" class="btn btn-primary mb-3">Добавить гостя</a>
        <form method="GET" action="/guests/" class="mb-3">
            <div class="row">
                <div class="col-md-6">
                    <input type="text" name="search" class="form-control" placeholder="Поиск по имени, телефону, email..." value=")" << escape_html(search) << R"(">
                </div>
                <div class="col-md-2">
                    <button type="submit" class="btn btn-primary">Поиск</button>
                </div>
            </div>
        </form>
    </div>
</div>

<div class="row">
    <div class="col-12">
        <table class="table table-striped">
            <thead>
                <tr>
                    <th>ФИО</th>
                    <th>Паспорт</th>
                    <th>Телефон</th>
                    <th>Email</th>
                    <th>Действия</th>
                </tr>
            </thead>
            <tbody>)";

        if (guests.empty()) {
            content << R"(
                <tr>
                    <td colspan="5" class="text-center text-muted">Гости не найдены</td>
                </tr>)";
        } else {
            for (const auto& guest : guests) {
                content << R"(
                <tr>
                    <td>)" << escape_html(guest.full_name()) << R"(</td>
                    <td>)" << escape_html(guest.passport_number) << R"(</td>
                    <td>)" << escape_html(guest.phone) << R"(</td>
                    <td>)" << escape_html(guest.email) << R"(</td>
                    <td><a href="/guests/)" << guest.guest_id << R"(/" class="btn btn-sm btn-primary">Подробнее</a></td>
                </tr>)";
            }
        }

        content << R"(
            </tbody>
        </table>
    </div>
</div>)";

        return base_template("Гости - Система бронирования отелей", content.str(), "", user);
    }

    static std::string guest_detail(Database& db, int64_t guest_id) {
        Guest guest = db.get_guest(guest_id);
        if (guest.guest_id == 0) {
            return base_template("Ошибка", "<div class='alert alert-danger'>Гость не найден</div>");
        }

        auto bookings = db.get_guest_bookings(guest_id);

        std::ostringstream content;
        content << R"(
<div class="row">
    <div class="col-12">
        <h1>)" << escape_html(guest.full_name()) << R"(</h1>
        <hr>
        <h3>Информация о госте</h3>
        <table class="table">
            <tr><th>Имя:</th><td>)" << escape_html(guest.first_name) << R"(</td></tr>
            <tr><th>Фамилия:</th><td>)" << escape_html(guest.last_name) << R"(</td></tr>
            <tr><th>Отчество:</th><td>)" << escape_html(guest.middle_name.empty() ? "-" : guest.middle_name) << R"(</td></tr>
            <tr><th>Паспорт:</th><td>)" << escape_html(guest.passport_number) << R"(</td></tr>
            <tr><th>Телефон:</th><td>)" << escape_html(guest.phone) << R"(</td></tr>
            <tr><th>Email:</th><td>)" << escape_html(guest.email.empty() ? "-" : guest.email) << R"(</td></tr>
        </table>

        <h3>Бронирования</h3>)";

        if (bookings.empty()) {
            content << R"(
        <p class="text-muted">Бронирований нет</p>)";
        } else {
            content << R"(
        <table class="table table-striped">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Номер</th>
                    <th>Заезд</th>
                    <th>Выезд</th>
                    <th>Стоимость</th>
                    <th>Действия</th>
                </tr>
            </thead>
            <tbody>)";
            for (const auto& booking : bookings) {
                Room room = db.get_room(booking.room_id);
                content << R"(
                <tr>
                    <td>)" << booking.booking_id << R"(</td>
                    <td>)" << escape_html(room.number) << R"(</td>
                    <td>)" << escape_html(booking.check_in_date) << R"(</td>
                    <td>)" << escape_html(booking.check_out_date) << R"(</td>
                    <td>)" << std::fixed << std::setprecision(2) << booking.total_price << R"( руб.</td>
                    <td><a href="/bookings/)" << booking.booking_id << R"(/" class="btn btn-sm btn-primary">Подробнее</a></td>
                </tr>)";
            }
            content << R"(
            </tbody>
        </table>)";
        }

        content << R"(
        <hr>
        <a href="/guests/" class="btn btn-secondary">Назад к списку</a>
    </div>
</div>)";

        return base_template("Гость " + guest.full_name() + " - Система бронирования отелей", content.str());
    }

    static std::string guest_form(const std::string& error = "", const Guest& guest = Guest()) {
        std::ostringstream content;
        content << R"(
<div class="row">
    <div class="col-md-8">
        <h1>Добавить гостя</h1>)";
        if (!error.empty()) {
            content << R"(
        <div class="alert alert-danger">)" << escape_html(error) << R"(</div>)";
        }
        content << R"(
        <form method="POST" action="/guests/create/">
            <div class="mb-3">
                <label class="form-label">Имя *</label>
                <input type="text" name="first_name" class="form-control" value=")" << escape_html(guest.first_name) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Фамилия *</label>
                <input type="text" name="last_name" class="form-control" value=")" << escape_html(guest.last_name) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Отчество</label>
                <input type="text" name="middle_name" class="form-control" value=")" << escape_html(guest.middle_name) << R"(">
            </div>
            <div class="mb-3">
                <label class="form-label">Номер паспорта *</label>
                <input type="text" name="passport_number" class="form-control" value=")" << escape_html(guest.passport_number) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Телефон *</label>
                <input type="text" name="phone" class="form-control" value=")" << escape_html(guest.phone) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Email</label>
                <input type="email" name="email" class="form-control" value=")" << escape_html(guest.email) << R"(">
            </div>
            <button type="submit" class="btn btn-primary">Сохранить</button>
            <a href="/guests/" class="btn btn-secondary">Отмена</a>
        </form>
    </div>
</div>)";

        return base_template("Добавить гостя - Система бронирования отелей", content.str());
    }

    static std::string bookings_list(Database& db, const std::string& search = "", const User* user = nullptr) {
        int64_t user_id = (user && user->user_id > 0) ? user->user_id : 0;
        auto bookings = db.get_all_bookings(search, user_id);

        std::ostringstream content;
        content << R"(
<div class="row mb-4">
    <div class="col-12">
        <h1>Бронирования</h1>
        <a href="/bookings/create/" class="btn btn-primary mb-3">Создать бронирование</a>
        <form method="GET" action="/bookings/" class="mb-3">
            <div class="row">
                <div class="col-md-6">
                    <input type="text" name="search" class="form-control" placeholder="Поиск по гостю, номеру..." value=")" << escape_html(search) << R"(">
                </div>
                <div class="col-md-2">
                    <button type="submit" class="btn btn-primary">Поиск</button>
                </div>
            </div>
        </form>
    </div>
</div>

<div class="row">
    <div class="col-12">
        <table class="table table-striped">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Гость</th>
                    <th>Номер</th>
                    <th>Заезд</th>
                    <th>Выезд</th>
                    <th>Стоимость</th>
                    <th>Действия</th>
                </tr>
            </thead>
            <tbody>)";

        if (bookings.empty()) {
            content << R"(
                <tr>
                    <td colspan="7" class="text-center text-muted">Бронирования не найдены</td>
                </tr>)";
        } else {
            for (const auto& booking : bookings) {
                Guest guest = db.get_guest(booking.guest_id);
                Room room = db.get_room(booking.room_id);
                content << R"(
                <tr>
                    <td>)" << booking.booking_id << R"(</td>
                    <td>)" << escape_html(guest.full_name()) << R"(</td>
                    <td>)" << escape_html(room.number) << R"(</td>
                    <td>)" << escape_html(booking.check_in_date) << R"(</td>
                    <td>)" << escape_html(booking.check_out_date) << R"(</td>
                    <td>)" << std::fixed << std::setprecision(2) << booking.total_price << R"( руб.</td>
                    <td><a href="/bookings/)" << booking.booking_id << R"(/" class="btn btn-sm btn-primary">Подробнее</a></td>
                </tr>)";
            }
        }

        content << R"(
            </tbody>
        </table>
    </div>
</div>)";

        return base_template("Бронирования - Система бронирования отелей", content.str(), "", user);
    }

    static std::string booking_detail(Database& db, int64_t booking_id) {
        Booking booking = db.get_booking(booking_id);
        if (booking.booking_id == 0) {
            return base_template("Ошибка", "<div class='alert alert-danger'>Бронирование не найдено</div>");
        }

        Guest guest = db.get_guest(booking.guest_id);
        Room room = db.get_room(booking.room_id);

        std::ostringstream content;
        content << R"(
<div class="row">
    <div class="col-12">
        <h1>Бронирование #)" << booking.booking_id << R"(</h1>
        <hr>
        <h3>Информация о бронировании</h3>
        <table class="table">
            <tr><th>Гость:</th><td><a href="/guests/)" << guest.guest_id << R"(/">)" << escape_html(guest.full_name()) << R"(</a></td></tr>
            <tr><th>Номер:</th><td><a href="/rooms/)" << room.room_id << R"(/">)" << escape_html(room.number) << " - " << escape_html(room.name) << R"(</a></td></tr>
            <tr><th>Дата заезда:</th><td>)" << escape_html(booking.check_in_date) << R"(</td></tr>
            <tr><th>Дата выезда:</th><td>)" << escape_html(booking.check_out_date) << R"(</td></tr>
            <tr><th>Взрослых:</th><td>)" << booking.adults_count << R"(</td></tr>
            <tr><th>Детей:</th><td>)" << booking.children_count << R"(</td></tr>
            <tr><th>Стоимость:</th><td>)" << std::fixed << std::setprecision(2) << booking.total_price << R"( руб.</td></tr>
            <tr><th>Пожелания:</th><td>)" << escape_html(booking.special_requests.empty() ? "-" : booking.special_requests) << R"(</td></tr>
        </table>
        <hr>
        <a href="/bookings/" class="btn btn-secondary">Назад к списку</a>
    </div>
</div>)";

        return base_template("Бронирование #" + std::to_string(booking_id) + " - Система бронирования отелей", content.str());
    }

    static std::string booking_form(Database& db, const std::string& error = "", const Booking& booking = Booking(), const Guest& guest = Guest(), int64_t user_id = 0) {
        auto rooms = db.get_all_rooms();
        auto guests = db.get_all_guests("", user_id);
        if (guests.size() > 10) {
            guests.resize(10);
        }

        std::ostringstream content;
        content << R"(
<div class="row">
    <div class="col-12">
        <h1>Создать бронирование</h1>)";
        if (!error.empty()) {
            content << R"(
        <div class="alert alert-danger">)" << escape_html(error) << R"(</div>)";
        }
        content << R"(
        <form method="POST" action="/bookings/create/">
            <div class="row">
                <div class="col-md-6">
                    <h3>Информация о госте</h3>
                    <div class="mb-3">
                        <label class="form-label">Выбрать существующего гостя</label>
                        <select name="guest_id" class="form-select">
                            <option value="">-- Выберите гостя --</option>)";
        for (const auto& g : guests) {
            content << R"(
                            <option value=")" << g.guest_id << R"(")" << (g.guest_id == booking.guest_id ? " selected" : "") << R"(>)" << escape_html(g.full_name()) << R"(</option>)";
        }
        content << R"(
                        </select>
                    </div>
                    <hr>
                    <h4>Или создать нового гостя</h4>
                    <div class="mb-3">
                        <label class="form-label">Имя *</label>
                        <input type="text" name="first_name" class="form-control" value=")" << escape_html(guest.first_name) << R"(">
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Фамилия *</label>
                        <input type="text" name="last_name" class="form-control" value=")" << escape_html(guest.last_name) << R"(">
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Отчество</label>
                        <input type="text" name="middle_name" class="form-control" value=")" << escape_html(guest.middle_name) << R"(">
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Номер паспорта *</label>
                        <input type="text" name="passport_number" class="form-control" value=")" << escape_html(guest.passport_number) << R"(">
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Телефон *</label>
                        <input type="text" name="phone" class="form-control" value=")" << escape_html(guest.phone) << R"(">
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Email</label>
                        <input type="email" name="email" class="form-control" value=")" << escape_html(guest.email) << R"(">
                    </div>
                </div>
                <div class="col-md-6">
                    <h3>Информация о бронировании</h3>
                    <div class="mb-3">
                        <label class="form-label">Номер *</label>
                        <select name="room_id" id="room_select" class="form-select" required onchange=\"updatePrice()\">)";
        for (const auto& room : rooms) {
            content << R"(
                            <option value=")" << room.room_id << R"(" data-price=")" << room.price_per_day << R"(")" << (room.room_id == booking.room_id ? " selected" : "") << R"(>)" << escape_html(room.number) << " - " << escape_html(room.name) << " (" << std::fixed << std::setprecision(2) << room.price_per_day << " руб./день)" << R"(</option>)";
        }
        content << R"(
                        </select>
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Дата заезда *</label>
                        <input type="date" name="check_in_date" id="check_in_date" class="form-control" value=")" << escape_html(booking.check_in_date) << R"(" required onchange=\"updatePrice()\">
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Дата выезда *</label>
                        <input type="date" name="check_out_date" id="check_out_date" class="form-control" value=")" << escape_html(booking.check_out_date) << R"(" required onchange=\"updatePrice()\">
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Количество взрослых *</label>
                        <input type="number" name="adults_count" class="form-control" value=")" << booking.adults_count << R"(" min="1" required>
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Количество детей</label>
                        <input type="number" name="children_count" class="form-control" value=")" << booking.children_count << R"(" min="0">
                    </div>
                    <div class="mb-3">
                        <div class="card bg-light">
                            <div class="card-body">
                                <h5>Информация о стоимости</h5>
                                <p class="mb-1"><strong>Цена за день:</strong> <span id="price_per_day">0</span> руб.</p>
                                <p class="mb-1"><strong>Количество дней:</strong> <span id="days_count">0</span></p>
                                <hr>
                                <p class="mb-0"><strong>Итоговая стоимость:</strong> <span id="total_price_display" class="text-primary fs-4">0</span> руб.</p>
                            </div>
                        </div>
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Пожелания</label>
                        <textarea name="special_requests" class="form-control" rows="4">)" << escape_html(booking.special_requests) << R"(</textarea>
                    </div>
                </div>
            </div>
            <button type="submit" class="btn btn-primary">Создать бронирование</button>
            <a href="/bookings/" class="btn btn-secondary">Отмена</a>
        </form>
    </div>
</div>
<script>
function updatePrice() {
    var roomSelect = document.getElementById('room_select');
    var checkIn = document.getElementById('check_in_date');
    var checkOut = document.getElementById('check_out_date');
    var pricePerDaySpan = document.getElementById('price_per_day');
    var daysCountSpan = document.getElementById('days_count');
    var totalPriceSpan = document.getElementById('total_price_display');
    
    if (!roomSelect || !checkIn || !checkOut) return;
    
    var selectedOption = roomSelect.options[roomSelect.selectedIndex];
    var pricePerDay = parseFloat(selectedOption.getAttribute('data-price')) || 0;
    
    pricePerDaySpan.textContent = pricePerDay.toFixed(2);
    
    if (checkIn.value && checkOut.value) {
        var checkInDate = new Date(checkIn.value);
        var checkOutDate = new Date(checkOut.value);
        
        if (checkOutDate > checkInDate) {
            var timeDiff = checkOutDate - checkInDate;
            var daysDiff = Math.ceil(timeDiff / (1000 * 60 * 60 * 24));
            daysCountSpan.textContent = daysDiff;
            
            var totalPrice = pricePerDay * daysDiff;
            totalPriceSpan.textContent = totalPrice.toFixed(2);
        } else {
            daysCountSpan.textContent = '0';
            totalPriceSpan.textContent = '0.00';
        }
    } else {
        daysCountSpan.textContent = '0';
        totalPriceSpan.textContent = '0.00';
    }
}

// Инициализация при загрузке страницы
document.addEventListener('DOMContentLoaded', function() {
    updatePrice();
});
</script>)";

        return base_template("Создать бронирование - Система бронирования отелей", content.str());
    }

    static std::string contact_page(const User* user = nullptr) {
        std::ostringstream content;
        content << R"(
<div class="row">
    <div class="col-12">
        <h1>Контакты</h1>
        <div class="card">
            <div class="card-body">
                <h3>Свяжитесь с нами</h3>
                <p><i class="bi bi-telephone"></i> Телефон: +7 (495) 123-45-67</p>
                <p><i class="bi bi-envelope"></i> Email: info@hotel-booking.ru</p>
                <p><i class="bi bi-geo-alt"></i> Адрес: г. Москва, ул. Примерная, д. 1</p>
                <p><i class="bi bi-clock"></i> Режим работы: Круглосуточно</p>
            </div>
        </div>
    </div>
</div>)";

        return base_template("Контакты - Система бронирования отелей", content.str(), "", user);
    }

    static std::string success_message(const std::string& message) {
        return "<div class='alert alert-success alert-dismissible fade show' role='alert'>" + escape_html(message) + 
               "<button type='button' class='btn-close' data-bs-dismiss='alert'></button></div>";
    }

    static std::string registration_form(const std::string& error = "", const User& user = User()) {
        std::ostringstream content;
        content << R"(
<div class="row justify-content-center">
    <div class="col-md-8">
        <h1>Регистрация</h1>)";
        if (!error.empty()) {
            content << R"(
        <div class="alert alert-danger">)" << escape_html(error) << R"(</div>)";
        }
        content << R"(
        <form method="POST" action="/register/" id="registrationForm">
            <div class="mb-3">
                <label class="form-label">Тип регистрации *</label>
                <select name="user_type" id="user_type" class="form-select" required>
                    <option value="user" )" << (user.user_type == "user" ? "selected" : "") << R"(>Пользователь</option>
                    <option value="organization" )" << (user.user_type == "organization" ? "selected" : "") << R"(>Организация</option>
                </select>
            </div>
            <div class="mb-3" id="organization_name_field" style="display: none;">
                <label class="form-label">Название организации *</label>
                <input type="text" name="organization_name" id="organization_name" class="form-control" value=")" << escape_html(user.organization_name) << R"(">
            </div>
            <div class="mb-3">
                <label class="form-label">ФИО *</label>
                <input type="text" name="full_name" class="form-control" value=")" << escape_html(user.full_name) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Номер телефона *</label>
                <input type="tel" name="phone" class="form-control" value=")" << escape_html(user.phone) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Email *</label>
                <input type="email" name="email" class="form-control" value=")" << escape_html(user.email) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Пароль *</label>
                <input type="password" name="password" class="form-control" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Подтверждение пароля *</label>
                <input type="password" name="password_confirm" class="form-control" required>
            </div>
            <button type="submit" class="btn btn-primary">Зарегистрироваться</button>
            <a href="/" class="btn btn-secondary">Отмена</a>
        </form>
    </div>
</div>
<script>
document.getElementById('user_type').addEventListener('change', function() {
    var orgField = document.getElementById('organization_name_field');
    var orgInput = document.getElementById('organization_name');
    if (this.value === 'organization') {
        orgField.style.display = 'block';
        orgInput.required = true;
    } else {
        orgField.style.display = 'none';
        orgInput.required = false;
    }
});
// Инициализация при загрузке страницы
var userType = document.getElementById('user_type');
if (userType.value === 'organization') {
    document.getElementById('organization_name_field').style.display = 'block';
    document.getElementById('organization_name').required = true;
}
</script>)";

        return base_template("Регистрация - Система бронирования отелей", content.str());
    }

    static std::string organization_dashboard(Database& db, int64_t organization_id, const User* user = nullptr) {
        auto hotels = db.get_hotels_by_organization(organization_id);
        User org = db.get_user(organization_id);
        
        std::ostringstream content;
        content << R"(
<div class="row mb-4">
    <div class="col-12">
        <h1>Панель организации</h1>
        <p class="lead">)" << escape_html(org.organization_name) << R"(</p>
    </div>
</div>

<div class="row mb-4">
    <div class="col-12">
        <a href="/hotels/create/" class="btn btn-primary">Создать отель</a>
        <a href="/organization/rooms/" class="btn btn-success">Управление номерами</a>
    </div>
</div>

<div class="row">
    <div class="col-12">
        <h2>Мои отели</h2>)";
        
        if (hotels.empty()) {
            content << R"(
        <p class="text-muted">У вас пока нет отелей. Создайте первый отель!</p>)";
        } else {
            for (const auto& hotel : hotels) {
                auto rooms = db.get_rooms_by_hotel(hotel.hotel_id);
                content << R"(
        <div class="card mb-3">
            <div class="card-body">
                <h3 class="card-title">)" << escape_html(hotel.name) << R"(</h3>
                <p class="card-text">)" << escape_html(hotel.description) << R"(</p>
                <p class="text-muted">Адрес: )" << escape_html(hotel.address) << R"(</p>
                <p class="text-muted">Номеров: )" << rooms.size() << R"(</p>
                <a href="/hotels/)" << hotel.hotel_id << R"(/rooms/create/" class="btn btn-primary">Добавить номер</a>
                <a href="/hotels/)" << hotel.hotel_id << R"(/bookings/" class="btn btn-info">Бронирования</a>
                <a href="/organization/dashboard/" class="btn btn-secondary">Назад</a>
            </div>
        </div>)";
            }
        }
        
        content << R"(
    </div>
</div>)";

        return base_template("Панель организации - Система бронирования отелей", content.str(), "", user);
    }

    static std::string hotel_form(int64_t organization_id, const std::string& error = "", const Hotel& hotel = Hotel(), const User* user = nullptr) {
        std::ostringstream content;
        content << R"(
<div class="row">
    <div class="col-md-8">
        <h1>Создать отель</h1>)";
        if (!error.empty()) {
            content << R"(
        <div class="alert alert-danger">)" << escape_html(error) << R"(</div>)";
        }
        content << R"(
        <form method="POST" action="/hotels/create/">
            <div class="mb-3">
                <label class="form-label">Название отеля *</label>
                <input type="text" name="name" class="form-control" value=")" << escape_html(hotel.name) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Описание</label>
                <textarea name="description" class="form-control" rows="4">)" << escape_html(hotel.description) << R"(</textarea>
            </div>
            <div class="mb-3">
                <label class="form-label">Адрес</label>
                <input type="text" name="address" class="form-control" value=")" << escape_html(hotel.address) << R"(">
            </div>
            <button type="submit" class="btn btn-primary">Создать отель</button>
            <a href="/organization/dashboard/" class="btn btn-secondary">Отмена</a>
        </form>
    </div>
</div>)";

        return base_template("Создать отель - Система бронирования отелей", content.str(), "", user);
    }

    static std::string room_form_for_hotel(Database& db, int64_t hotel_id, int64_t organization_id, const std::string& error = "", const Room& room = Room(), const User* user = nullptr) {
        std::ostringstream content;
        content << R"(
<div class="row">
    <div class="col-md-8">
        <h1>Добавить номер в отель</h1>)";
        if (!error.empty()) {
            content << R"(
        <div class="alert alert-danger">)" << escape_html(error) << R"(</div>)";
        }
        content << R"(
        <form method="POST" action="/hotels/)" << hotel_id << R"(/rooms/create/">
            <div class="mb-3">
                <label class="form-label">Номер комнаты *</label>
                <input type="text" name="number" class="form-control" value=")" << escape_html(room.number) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Название *</label>
                <input type="text" name="name" class="form-control" value=")" << escape_html(room.name) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Описание</label>
                <textarea name="description" class="form-control" rows="4">)" << escape_html(room.description) << R"(</textarea>
            </div>
            <div class="mb-3">
                <label class="form-label">Тип номера *</label>
                <input type="text" name="type_name" class="form-control" value=")" << escape_html(room.type_name) << R"(" required placeholder="Например: Стандарт, Люкс, Сьют">
            </div>
            <div class="mb-3">
                <label class="form-label">Цена за день (руб.) *</label>
                <input type="number" name="price_per_day" class="form-control" value=")" << (room.price_per_day > 0 ? std::to_string(room.price_per_day) : "") << R"(" step="0.01" min="0" required>
            </div>
            <button type="submit" class="btn btn-primary">Добавить номер</button>
            <a href="/organization/dashboard/" class="btn btn-secondary">Отмена</a>
        </form>
    </div>
</div>)";

        return base_template("Добавить номер - Система бронирования отелей", content.str(), "", user);
    }

    static std::string profile_page(const User& user, const std::string& error = "", const std::string& success = "") {
        std::ostringstream content;
        content << R"(
<div class="row justify-content-center">
    <div class="col-md-8">
        <h1>Мой профиль</h1>)";
        
        if (!error.empty()) {
            content << R"(
        <div class="alert alert-danger">)" << escape_html(error) << R"(</div>)";
        }
        
        if (!success.empty()) {
            content << R"(
        <div class="alert alert-success">)" << escape_html(success) << R"(</div>)";
        }
        
        content << R"(
        <div class="card mb-4">
            <div class="card-body">
                <h3>Информация о пользователе</h3>
                <table class="table">
                    <tr><th>Тип:</th><td>)" << escape_html(user.user_type == "organization" ? "Организация" : "Пользователь") << R"(</td></tr>)";
        
        if (user.is_organization()) {
            content << R"(
                    <tr><th>Название организации:</th><td>)" << escape_html(user.organization_name) << R"(</td></tr>)";
        }
        
        content << R"(
                    <tr><th>ФИО:</th><td>)" << escape_html(user.full_name) << R"(</td></tr>
                    <tr><th>Телефон:</th><td>)" << escape_html(user.phone) << R"(</td></tr>
                    <tr><th>Email:</th><td>)" << escape_html(user.email) << R"(</td></tr>
                    <tr><th>Дата регистрации:</th><td>)" << escape_html(user.created_at) << R"(</td></tr>
                </table>
            </div>
        </div>

        )";
        
        if (!user.is_organization()) {
            content << R"(
        <div class="card mb-4">
            <div class="card-body">
                <h3>Мои бронирования</h3>
                <p>Просмотрите и управляйте своими бронированиями</p>
                <a href="/my-bookings/" class="btn btn-primary">Перейти к бронированиям</a>
            </div>
        </div>
        )";
        }
        
        content << R"(

        <div class="card">
            <div class="card-body">
                <h3>Изменить данные</h3>
                <form method="POST" action="/profile/">
                    <div class="mb-3">
                        <label class="form-label">ФИО *</label>
                        <input type="text" name="full_name" class="form-control" value=")" << escape_html(user.full_name) << R"(" required>
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Номер телефона *</label>
                        <input type="tel" name="phone" class="form-control" value=")" << escape_html(user.phone) << R"(" required>
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Email *</label>
                        <input type="email" name="email" class="form-control" value=")" << escape_html(user.email) << R"(" required>
                    </div>)";
        
        if (user.is_organization()) {
            content << R"(
                    <div class="mb-3">
                        <label class="form-label">Название организации *</label>
                        <input type="text" name="organization_name" class="form-control" value=")" << escape_html(user.organization_name) << R"(" required>
                    </div>)";
        }
        
        content << R"(
                    <button type="submit" class="btn btn-primary">Сохранить изменения</button>
                </form>
            </div>
        </div>

        <div class="card mt-4">
            <div class="card-body">
                <h3>Изменить пароль</h3>
                <form method="POST" action="/profile/password/">
                    <div class="mb-3">
                        <label class="form-label">Текущий пароль</label>
                        <input type="password" name="current_password" class="form-control" required>
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Новый пароль</label>
                        <input type="password" name="new_password" class="form-control" required>
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Подтверждение нового пароля</label>
                        <input type="password" name="new_password_confirm" class="form-control" required>
                    </div>
                    <button type="submit" class="btn btn-primary">Изменить пароль</button>
                </form>
            </div>
        </div>
    </div>
</div>)";

        return base_template("Профиль - Система бронирования отелей", content.str(), "", &user);
    }

    static std::string login_form(const std::string& error = "") {
        std::ostringstream content;
        content << R"(
<div class="row justify-content-center">
    <div class="col-md-6">
        <h1>Вход в систему</h1>)";
        if (!error.empty()) {
            content << R"(
        <div class="alert alert-danger">)" << escape_html(error) << R"(</div>)";
        }
        content << R"(
        <form method="POST" action="/login/">
            <div class="mb-3">
                <label class="form-label">Email *</label>
                <input type="email" name="email" class="form-control" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Пароль *</label>
                <input type="password" name="password" class="form-control" required>
            </div>
            <button type="submit" class="btn btn-primary">Войти</button>
            <a href="/register/" class="btn btn-link">Нет аккаунта? Зарегистрироваться</a>
        </form>
    </div>
</div>)";

        return base_template("Вход - Система бронирования отелей", content.str());
    }

    // Страницы для управления номерами организации
    static std::string organization_rooms_list(Database& db, int64_t organization_id, const std::string& error = "", const std::string& success = "", const User* user = nullptr) {
        auto hotels = db.get_hotels_by_organization(organization_id);
        
        std::ostringstream content;
        content << R"(
<div class="row mb-4">
    <div class="col-12">
        <h1>Мои номера</h1>)";
        
        if (!error.empty()) {
            content << R"(
        <div class="alert alert-danger">)" << escape_html(error) << R"(</div>)";
        }
        
        if (!success.empty()) {
            content << R"(
        <div class="alert alert-success">)" << escape_html(success) << R"(</div>)";
        }
        
        content << R"(
    </div>
</div>

<div class="row">)";
        
        if (hotels.empty()) {
            content << R"(
    <div class="col-12">
        <p class="text-muted">У вас пока нет отелей. Создайте отель и добавьте номера!</p>
    </div>)";
        } else {
            for (const auto& hotel : hotels) {
                auto rooms = db.get_rooms_by_hotel(hotel.hotel_id);
                content << R"(
    <div class="col-12 mb-4">
        <div class="card">
            <div class="card-header">
                <h3>)" << escape_html(hotel.name) << R"(</h3>
            </div>
            <div class="card-body">)";
                
                if (rooms.empty()) {
                    content << R"(
                <p class="text-muted">В этом отеле пока нет номеров.</p>
                <a href="/hotels/)" << hotel.hotel_id << R"(/rooms/create/" class="btn btn-primary">Добавить номер</a>)";
                } else {
                    content << R"(
                <table class="table">
                    <thead>
                        <tr>
                            <th>Номер</th>
                            <th>Название</th>
                            <th>Тип</th>
                            <th>Цена за день</th>
                            <th>Действия</th>
                        </tr>
                    </thead>
                    <tbody>)";
                    for (const auto& room : rooms) {
                        content << R"(
                        <tr>
                            <td>)" << escape_html(room.number) << R"(</td>
                            <td>)" << escape_html(room.name) << R"(</td>
                            <td>)" << escape_html(room.type_name) << R"(</td>
                            <td>)" << std::fixed << std::setprecision(2) << room.price_per_day << R"( руб.</td>
                            <td>
                                <a href="/rooms/)" << room.room_id << R"(/edit/" class="btn btn-sm btn-primary">Редактировать</a>
                                <a href="/rooms/)" << room.room_id << R"(/delete/" class="btn btn-sm btn-danger" onclick=\"return confirm('Вы уверены, что хотите удалить этот номер?')\">Удалить</a>
                            </td>
                        </tr>)";
                    }
                    content << R"(
                    </tbody>
                </table>
                <a href="/hotels/)" << hotel.hotel_id << R"(/rooms/create/" class="btn btn-primary">Добавить номер</a>)";
                }
                content << R"(
            </div>
        </div>
    </div>)";
            }
        }
        
        content << R"(
</div>
<div class="row mt-4">
    <div class="col-12">
        <a href="/organization/dashboard/" class="btn btn-secondary">Назад к панели</a>
    </div>
</div>)";

        return base_template("Мои номера - Система бронирования отелей", content.str(), "", user);
    }

    static std::string room_edit_form(Database& db, int64_t room_id, const std::string& error = "", const Room& room = Room(), const User* user = nullptr) {
        Room room_data = room.room_id == 0 ? db.get_room(room_id) : room;
        if (room_data.room_id == 0) {
            return base_template("Ошибка", "<div class='alert alert-danger'>Номер не найден</div>", "", user);
        }
        
        std::ostringstream content;
        content << R"(
<div class="row">
    <div class="col-md-8">
        <h1>Редактировать номер</h1>)";
        if (!error.empty()) {
            content << R"(
        <div class="alert alert-danger">)" << escape_html(error) << R"(</div>)";
        }
        content << R"(
        <form method="POST" action="/rooms/)" << room_id << R"(/edit/">
            <div class="mb-3">
                <label class="form-label">Номер комнаты *</label>
                <input type="text" name="number" class="form-control" value=")" << escape_html(room_data.number) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Название *</label>
                <input type="text" name="name" class="form-control" value=")" << escape_html(room_data.name) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Описание</label>
                <textarea name="description" class="form-control" rows="4">)" << escape_html(room_data.description) << R"(</textarea>
            </div>
            <div class="mb-3">
                <label class="form-label">Тип номера *</label>
                <input type="text" name="type_name" class="form-control" value=")" << escape_html(room_data.type_name) << R"(" required>
            </div>
            <div class="mb-3">
                <label class="form-label">Цена за день (руб.) *</label>
                <input type="number" name="price_per_day" class="form-control" value=")" << std::fixed << std::setprecision(2) << room_data.price_per_day << R"(" step="0.01" min="0" required>
            </div>
            <button type="submit" class="btn btn-primary">Сохранить изменения</button>
            <a href="/organization/rooms/" class="btn btn-secondary">Отмена</a>
        </form>
    </div>
</div>)";

        return base_template("Редактировать номер - Система бронирования отелей", content.str(), "", user);
    }

    static std::string hotel_bookings_list(Database& db, int64_t hotel_id, const std::string& error = "", const std::string& success = "", const User* user = nullptr) {
        Hotel hotel = db.get_hotel(hotel_id);
        if (hotel.hotel_id == 0) {
            return base_template("Ошибка", "<div class='alert alert-danger'>Отель не найден</div>", "", user);
        }
        
        auto bookings = db.get_bookings_by_hotel(hotel_id);
        
        std::ostringstream content;
        content << R"(
<div class="row mb-4">
    <div class="col-12">
        <h1>Бронирования отеля: )" << escape_html(hotel.name) << R"(</h1>)";
        
        if (!error.empty()) {
            content << R"(
        <div class="alert alert-danger">)" << escape_html(error) << R"(</div>)";
        }
        
        if (!success.empty()) {
            content << R"(
        <div class="alert alert-success">)" << escape_html(success) << R"(</div>)";
        }
        
        content << R"(
    </div>
</div>

<div class="row">
    <div class="col-12">
        <table class="table table-striped">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Гость</th>
                    <th>Номер</th>
                    <th>Заезд</th>
                    <th>Выезд</th>
                    <th>Стоимость</th>
                    <th>Действия</th>
                </tr>
            </thead>
            <tbody>)";
        
        if (bookings.empty()) {
            content << R"(
                <tr>
                    <td colspan="7" class="text-center text-muted">Бронирования не найдены</td>
                </tr>)";
        } else {
            for (const auto& booking : bookings) {
                Guest guest = db.get_guest(booking.guest_id);
                Room room = db.get_room(booking.room_id);
                content << R"(
                <tr>
                    <td>)" << booking.booking_id << R"(</td>
                    <td>)" << escape_html(guest.full_name()) << R"(</td>
                    <td>)" << escape_html(room.number) << R"(</td>
                    <td>)" << escape_html(booking.check_in_date) << R"(</td>
                    <td>)" << escape_html(booking.check_out_date) << R"(</td>
                    <td>)" << std::fixed << std::setprecision(2) << booking.total_price << R"( руб.</td>
                    <td>
                        <a href="/bookings/)" << booking.booking_id << R"(/edit/" class="btn btn-sm btn-primary">Редактировать</a>
                        <a href="/bookings/)" << booking.booking_id << R"(/" class="btn btn-sm btn-secondary">Подробнее</a>
                    </td>
                </tr>)";
            }
        }
        
        content << R"(
            </tbody>
        </table>
    </div>
</div>

<div class="row mt-4">
    <div class="col-12">
        <a href="/organization/dashboard/" class="btn btn-secondary">Назад к панели</a>
    </div>
</div>)";

        return base_template("Бронирования отеля - Система бронирования отелей", content.str(), "", user);
    }

    static std::string booking_edit_form(Database& db, int64_t booking_id, const std::string& error = "", const Booking& booking = Booking(), const User* user = nullptr) {
        Booking booking_data = booking.booking_id == 0 ? db.get_booking(booking_id) : booking;
        if (booking_data.booking_id == 0) {
            return base_template("Ошибка", "<div class='alert alert-danger'>Бронирование не найдено</div>", "", user);
        }
        
        Guest guest = db.get_guest(booking_data.guest_id);
        Room room = db.get_room(booking_data.room_id);
        auto rooms = db.get_rooms_by_hotel(room.hotel_id);
        
        std::ostringstream content;
        content << R"(
<div class="row">
    <div class="col-12">
        <h1>Редактировать бронирование</h1>)";
        if (!error.empty()) {
            content << R"(
        <div class="alert alert-danger">)" << escape_html(error) << R"(</div>)";
        }
        content << R"(
        <form method="POST" action="/bookings/)" << booking_id << R"(/edit/">
            <div class="row">
                <div class="col-md-6">
                    <h3>Информация о госте</h3>
                    <p><strong>Гость:</strong> )" << escape_html(guest.full_name()) << R"(</p>
                    <p><strong>Телефон:</strong> )" << escape_html(guest.phone) << R"(</p>
                    <p><strong>Email:</strong> )" << escape_html(guest.email) << R"(</p>
                </div>
                <div class="col-md-6">
                    <h3>Информация о бронировании</h3>
                    <div class="mb-3">
                        <label class="form-label">Номер *</label>
                        <select name="room_id" class="form-select" required>)";
        for (const auto& r : rooms) {
            content << R"(
                            <option value=")" << r.room_id << R"(")" << (r.room_id == booking_data.room_id ? " selected" : "") << R"(>)" << escape_html(r.number) << " - " << escape_html(r.name) << R"(</option>)";
        }
        content << R"(
                        </select>
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Дата заезда *</label>
                        <input type="date" name="check_in_date" class="form-control" value=")" << escape_html(booking_data.check_in_date) << R"(" required>
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Дата выезда *</label>
                        <input type="date" name="check_out_date" class="form-control" value=")" << escape_html(booking_data.check_out_date) << R"(" required>
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Количество взрослых *</label>
                        <input type="number" name="adults_count" class="form-control" value=")" << booking_data.adults_count << R"(" min="1" required>
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Количество детей</label>
                        <input type="number" name="children_count" class="form-control" value=")" << booking_data.children_count << R"(" min="0">
                    </div>
                    <div class="mb-3">
                        <label class="form-label">Пожелания</label>
                        <textarea name="special_requests" class="form-control" rows="4">)" << escape_html(booking_data.special_requests) << R"(</textarea>
                    </div>
                </div>
            </div>
            <button type="submit" class="btn btn-primary">Сохранить изменения</button>
            <a href="/hotels/)" << room.hotel_id << R"(/bookings/" class="btn btn-secondary">Отмена</a>
        </form>
    </div>
</div>)";

        return base_template("Редактировать бронирование - Система бронирования отелей", content.str(), "", user);
    }

    static std::string user_bookings_list(Database& db, int64_t user_id, const std::string& error = "", const std::string& success = "", const User* user = nullptr) {
        auto bookings = db.get_bookings_by_user(user_id);
        
        std::ostringstream content;
        content << R"(
<div class="row mb-4">
    <div class="col-12">
        <h1>Мои бронирования</h1>)";
        
        if (!error.empty()) {
            content << R"(
        <div class="alert alert-danger">)" << escape_html(error) << R"(</div>)";
        }
        
        if (!success.empty()) {
            content << R"(
        <div class="alert alert-success">)" << escape_html(success) << R"(</div>)";
        }
        
        content << R"(
    </div>
</div>

<div class="row">
    <div class="col-12">
        <table class="table table-striped">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Гость</th>
                    <th>Номер</th>
                    <th>Заезд</th>
                    <th>Выезд</th>
                    <th>Стоимость</th>
                    <th>Действия</th>
                </tr>
            </thead>
            <tbody>)";
        
        if (bookings.empty()) {
            content << R"(
                <tr>
                    <td colspan="7" class="text-center text-muted">У вас пока нет бронирований</td>
                </tr>)";
        } else {
            for (const auto& booking : bookings) {
                Guest guest = db.get_guest(booking.guest_id);
                Room room = db.get_room(booking.room_id);
                content << R"(
                <tr>
                    <td>)" << booking.booking_id << R"(</td>
                    <td>)" << escape_html(guest.full_name()) << R"(</td>
                    <td>)" << escape_html(room.number) << R"(</td>
                    <td>)" << escape_html(booking.check_in_date) << R"(</td>
                    <td>)" << escape_html(booking.check_out_date) << R"(</td>
                    <td>)" << std::fixed << std::setprecision(2) << booking.total_price << R"( руб.</td>
                    <td>
                        <a href="/bookings/)" << booking.booking_id << R"(/" class="btn btn-sm btn-primary">Подробнее</a>
                        <a href="/bookings/)" << booking.booking_id << R"(/cancel/" class="btn btn-sm btn-danger" onclick=\"return confirm('Вы уверены, что хотите отменить это бронирование?')\">Отменить</a>
                    </td>
                </tr>)";
            }
        }
        
        content << R"(
            </tbody>
        </table>
    </div>
</div>

<div class="row mt-4">
    <div class="col-12">
        <a href="/profile/" class="btn btn-secondary">Назад к профилю</a>
    </div>
</div>)";

        return base_template("Мои бронирования - Система бронирования отелей", content.str(), "", user);
    }
};

#endif // HTML_GENERATOR_H

