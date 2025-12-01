#include "../include/models.h"
#include "../include/database.h"
#include "../include/html_generator.h"
#include <httplib.h>
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <ctime>
#include <cmath>

using namespace httplib;

std::map<std::string, std::string> parse_form_data(const std::string& body) {
    std::map<std::string, std::string> params;
    std::istringstream iss(body);
    std::string pair;
    
    while (std::getline(iss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            
            // URL decode
            std::string decoded_key, decoded_value;
            for (size_t i = 0; i < key.length(); ++i) {
                if (key[i] == '%' && i + 2 < key.length()) {
                    int hex = 0;
                    std::istringstream(key.substr(i + 1, 2)) >> std::hex >> hex;
                    decoded_key += static_cast<char>(hex);
                    i += 2;
                } else if (key[i] == '+') {
                    decoded_key += ' ';
                } else {
                    decoded_key += key[i];
                }
            }
            
            for (size_t i = 0; i < value.length(); ++i) {
                if (value[i] == '%' && i + 2 < value.length()) {
                    int hex = 0;
                    std::istringstream(value.substr(i + 1, 2)) >> std::hex >> hex;
                    decoded_value += static_cast<char>(hex);
                    i += 2;
                } else if (value[i] == '+') {
                    decoded_value += ' ';
                } else {
                    decoded_value += value[i];
                }
            }
            
            params[decoded_key] = decoded_value;
        }
    }
    
    return params;
}

std::string url_decode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int hex = 0;
            std::istringstream(str.substr(i + 1, 2)) >> std::hex >> hex;
            result += static_cast<char>(hex);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

bool validate_date(const std::string& date) {
    if (date.length() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;
    return true;
}

bool date_less(const std::string& date1, const std::string& date2) {
    return date1 < date2;
}

// Функции для работы с сессиями
int64_t get_user_id_from_session(const Request& req) {
    if (req.has_header("Cookie")) {
        std::string cookies = req.get_header_value("Cookie");
        size_t pos = cookies.find("user_id=");
        if (pos != std::string::npos) {
            size_t start = pos + 8; // "user_id=" length
            size_t end = cookies.find(";", start);
            if (end == std::string::npos) {
                end = cookies.length();
            }
            std::string user_id_str = cookies.substr(start, end - start);
            try {
                return std::stoll(user_id_str);
            } catch (...) {
                return 0;
            }
        }
    }
    return 0;
}

void set_user_session(Response& res, int64_t user_id) {
    res.set_header("Set-Cookie", "user_id=" + std::to_string(user_id) + "; Path=/; HttpOnly");
}

void clear_user_session(Response& res) {
    res.set_header("Set-Cookie", "user_id=; Path=/; HttpOnly; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
}

int main() {
    try {
        Database db("hotels.db");
        Server svr;

        // Вспомогательная функция для получения пользователя из сессии
        auto get_user_from_session = [&db](const Request& req) -> User {
            int64_t user_id = get_user_id_from_session(req);
            if (user_id != 0) {
                return db.get_user(user_id);
            }
            return User();
        };

        // Главная страница
        svr.Get("/", [&db, &get_user_from_session](const Request& req, Response& res) {
            User user = get_user_from_session(req);
            res.set_content(HtmlGenerator::home_page(db, &user), "text/html; charset=utf-8");
        });

        // Список номеров
        svr.Get("/rooms/", [&db](const Request& req, Response& res) {
            std::string type_filter = "";
            if (req.has_param("type")) {
                type_filter = url_decode(req.get_param_value("type"));
            }
            res.set_content(HtmlGenerator::rooms_list(db, type_filter), "text/html; charset=utf-8");
        });

        // Детали номера
        svr.Get(R"(/rooms/(\d+)/)", [&db](const Request& req, Response& res) {
            int64_t room_id = std::stoll(req.matches[1]);
            std::string check_in = "";
            std::string check_out = "";
            if (req.has_param("check_in")) {
                check_in = url_decode(req.get_param_value("check_in"));
            }
            if (req.has_param("check_out")) {
                check_out = url_decode(req.get_param_value("check_out"));
            }
            res.set_content(HtmlGenerator::room_detail(db, room_id, check_in, check_out), "text/html; charset=utf-8");
        });

        // Список гостей
        svr.Get("/guests/", [&db](const Request& req, Response& res) {
            std::string search = "";
            if (req.has_param("search")) {
                search = url_decode(req.get_param_value("search"));
            }
            res.set_content(HtmlGenerator::guests_list(db, search), "text/html; charset=utf-8");
        });

        // Форма создания гостя (GET)
        svr.Get("/guests/create/", [&db](const Request& req, Response& res) {
            res.set_content(HtmlGenerator::guest_form(), "text/html; charset=utf-8");
        });

        // Создание гостя (POST)
        svr.Post("/guests/create/", [&db](const Request& req, Response& res) {
            auto params = parse_form_data(req.body);
            
            Guest guest;
            guest.first_name = params.count("first_name") ? params["first_name"] : "";
            guest.last_name = params.count("last_name") ? params["last_name"] : "";
            guest.middle_name = params.count("middle_name") ? params["middle_name"] : "";
            guest.passport_number = params.count("passport_number") ? params["passport_number"] : "";
            guest.email = params.count("email") ? params["email"] : "";
            guest.phone = params.count("phone") ? params["phone"] : "";

            if (guest.first_name.empty() || guest.last_name.empty() || 
                guest.passport_number.empty() || guest.phone.empty()) {
                std::string error = "Заполните все обязательные поля";
                res.set_content(HtmlGenerator::guest_form(error, guest), "text/html; charset=utf-8");
                return;
            }

            try {
                int64_t guest_id = db.create_guest(guest);
                res.set_header("Location", "/guests/");
                res.status = 302;
            } catch (const std::exception& e) {
                std::string error = "Ошибка при создании гостя: " + std::string(e.what());
                res.set_content(HtmlGenerator::guest_form(error, guest), "text/html; charset=utf-8");
            }
        });

        // Детали гостя
        svr.Get(R"(/guests/(\d+)/)", [&db](const Request& req, Response& res) {
            int64_t guest_id = std::stoll(req.matches[1]);
            res.set_content(HtmlGenerator::guest_detail(db, guest_id), "text/html; charset=utf-8");
        });

        // Список бронирований
        svr.Get("/bookings/", [&db](const Request& req, Response& res) {
            std::string search = "";
            if (req.has_param("search")) {
                search = url_decode(req.get_param_value("search"));
            }
            res.set_content(HtmlGenerator::bookings_list(db, search), "text/html; charset=utf-8");
        });

        // Форма создания бронирования (GET)
        svr.Get("/bookings/create/", [&db](const Request& req, Response& res) {
            Booking booking;
            if (req.has_param("room")) {
                booking.room_id = std::stoll(url_decode(req.get_param_value("room")));
            }
            if (req.has_param("check_in")) {
                booking.check_in_date = url_decode(req.get_param_value("check_in"));
            }
            if (req.has_param("check_out")) {
                booking.check_out_date = url_decode(req.get_param_value("check_out"));
            }
            res.set_content(HtmlGenerator::booking_form(db, "", booking), "text/html; charset=utf-8");
        });

        // Создание бронирования (POST)
        svr.Post("/bookings/create/", [&db](const Request& req, Response& res) {
            auto params = parse_form_data(req.body);
            
            Booking booking;
            Guest guest;
            
            // Проверяем, выбран ли существующий гость
            std::string guest_id_str = params.count("guest_id") ? params["guest_id"] : "";
            int64_t guest_id = 0;
            
            if (!guest_id_str.empty()) {
                try {
                    guest_id = std::stoll(guest_id_str);
                    guest = db.get_guest(guest_id);
                    if (guest.guest_id == 0) {
                        res.set_content(HtmlGenerator::booking_form(db, "Выбранный гость не найден", booking, guest), "text/html; charset=utf-8");
                        return;
                    }
                } catch (...) {
                    res.set_content(HtmlGenerator::booking_form(db, "Неверный ID гостя", booking, guest), "text/html; charset=utf-8");
                    return;
                }
            } else {
                // Создаем нового гостя
                guest.first_name = params.count("first_name") ? params["first_name"] : "";
                guest.last_name = params.count("last_name") ? params["last_name"] : "";
                guest.middle_name = params.count("middle_name") ? params["middle_name"] : "";
                guest.passport_number = params.count("passport_number") ? params["passport_number"] : "";
                guest.email = params.count("email") ? params["email"] : "";
                guest.phone = params.count("phone") ? params["phone"] : "";

                if (guest.first_name.empty() || guest.last_name.empty() || 
                    guest.passport_number.empty() || guest.phone.empty()) {
                    std::string error = "Заполните все обязательные поля гостя";
                    res.set_content(HtmlGenerator::booking_form(db, error, booking, guest), "text/html; charset=utf-8");
                    return;
                }

                try {
                    guest_id = db.create_guest(guest);
                } catch (const std::exception& e) {
                    std::string error = "Ошибка при создании гостя: " + std::string(e.what());
                    res.set_content(HtmlGenerator::booking_form(db, error, booking, guest), "text/html; charset=utf-8");
                    return;
                }
            }

            // Заполняем данные бронирования
            std::string room_id_str = params.count("room_id") ? params["room_id"] : "";
            if (room_id_str.empty()) {
                res.set_content(HtmlGenerator::booking_form(db, "Выберите номер", booking, guest), "text/html; charset=utf-8");
                return;
            }
            
            try {
                booking.room_id = std::stoll(room_id_str);
                booking.guest_id = guest_id;
                booking.check_in_date = params.count("check_in_date") ? params["check_in_date"] : "";
                booking.check_out_date = params.count("check_out_date") ? params["check_out_date"] : "";
                
                if (booking.check_in_date.empty() || booking.check_out_date.empty()) {
                    res.set_content(HtmlGenerator::booking_form(db, "Укажите даты заезда и выезда", booking, guest), "text/html; charset=utf-8");
                    return;
                }

                // Валидация дат
                if (!date_less(booking.check_in_date, booking.check_out_date)) {
                    res.set_content(HtmlGenerator::booking_form(db, "Дата заезда должна быть раньше даты выезда", booking, guest), "text/html; charset=utf-8");
                    return;
                }

                if (date_less(booking.check_in_date, get_current_date())) {
                    res.set_content(HtmlGenerator::booking_form(db, "Дата заезда не может быть в прошлом", booking, guest), "text/html; charset=utf-8");
                    return;
                }

                // Проверка доступности номера
                if (!db.is_room_available(booking.room_id, booking.check_in_date, booking.check_out_date)) {
                    res.set_content(HtmlGenerator::booking_form(db, "Номер занят на выбранные даты", booking, guest), "text/html; charset=utf-8");
                    return;
                }

                std::string adults_str = params.count("adults_count") ? params["adults_count"] : "1";
                booking.adults_count = std::stoi(adults_str);
                if (booking.adults_count < 1) {
                    booking.adults_count = 1;
                }

                std::string children_str = params.count("children_count") ? params["children_count"] : "0";
                booking.children_count = std::stoi(children_str);
                if (booking.children_count < 0) {
                    booking.children_count = 0;
                }

                // Получаем номер для расчета стоимости
                Room room = db.get_room(booking.room_id);
                if (room.room_id == 0) {
                    res.set_content(HtmlGenerator::booking_form(db, "Номер не найден", booking, guest), "text/html; charset=utf-8");
                    return;
                }

                // Рассчитываем количество дней
                std::tm check_in_tm = {}, check_out_tm = {};
                std::istringstream check_in_ss(booking.check_in_date);
                std::istringstream check_out_ss(booking.check_out_date);
                check_in_ss >> std::get_time(&check_in_tm, "%Y-%m-%d");
                check_out_ss >> std::get_time(&check_out_tm, "%Y-%m-%d");
                
                std::time_t check_in_time = std::mktime(&check_in_tm);
                std::time_t check_out_time = std::mktime(&check_out_tm);
                double days_diff = std::difftime(check_out_time, check_in_time) / (60 * 60 * 24);
                int days = static_cast<int>(std::ceil(days_diff));
                if (days < 1) days = 1;

                // Рассчитываем итоговую стоимость: цена за день × количество дней
                booking.total_price = room.price_per_day * days;

                booking.special_requests = params.count("special_requests") ? params["special_requests"] : "";

                db.create_booking(booking);
                res.set_header("Location", "/bookings/");
                res.status = 302;
            } catch (const std::exception& e) {
                std::string error = "Ошибка при создании бронирования: " + std::string(e.what());
                res.set_content(HtmlGenerator::booking_form(db, error, booking, guest), "text/html; charset=utf-8");
            }
        });

        // Детали бронирования
        svr.Get(R"(/bookings/(\d+)/)", [&db](const Request& req, Response& res) {
            int64_t booking_id = std::stoll(req.matches[1]);
            res.set_content(HtmlGenerator::booking_detail(db, booking_id), "text/html; charset=utf-8");
        });

        // Регистрация (GET)
        svr.Get("/register/", [&db](const Request& req, Response& res) {
            res.set_content(HtmlGenerator::registration_form(), "text/html; charset=utf-8");
        });

        // Регистрация (POST)
        svr.Post("/register/", [&db](const Request& req, Response& res) {
            auto params = parse_form_data(req.body);
            
            User user;
            user.user_type = params.count("user_type") ? params["user_type"] : "";
            user.full_name = params.count("full_name") ? params["full_name"] : "";
            user.phone = params.count("phone") ? params["phone"] : "";
            user.email = params.count("email") ? params["email"] : "";
            std::string password = params.count("password") ? params["password"] : "";
            std::string password_confirm = params.count("password_confirm") ? params["password_confirm"] : "";
            
            if (user.user_type == "organization") {
                user.organization_name = params.count("organization_name") ? params["organization_name"] : "";
            }

            // Валидация
            if (user.full_name.empty() || user.phone.empty() || user.email.empty() || password.empty()) {
                std::string error = "Заполните все обязательные поля";
                res.set_content(HtmlGenerator::registration_form(error, user), "text/html; charset=utf-8");
                return;
            }

            if (user.user_type != "user" && user.user_type != "organization") {
                std::string error = "Выберите тип регистрации";
                res.set_content(HtmlGenerator::registration_form(error, user), "text/html; charset=utf-8");
                return;
            }

            if (user.user_type == "organization" && user.organization_name.empty()) {
                std::string error = "Укажите название организации";
                res.set_content(HtmlGenerator::registration_form(error, user), "text/html; charset=utf-8");
                return;
            }

            if (password != password_confirm) {
                std::string error = "Пароли не совпадают";
                res.set_content(HtmlGenerator::registration_form(error, user), "text/html; charset=utf-8");
                return;
            }

            // Проверка, существует ли пользователь с таким email
            User existing = db.get_user_by_email(user.email);
            if (existing.user_id != 0) {
                std::string error = "Пользователь с таким email уже зарегистрирован";
                res.set_content(HtmlGenerator::registration_form(error, user), "text/html; charset=utf-8");
                return;
            }

            user.password = password; // В реальном приложении здесь должно быть хеширование пароля

            try {
                int64_t user_id = db.create_user(user);
                set_user_session(res, user_id);
                if (user.user_type == "organization") {
                    res.set_header("Location", "/organization/dashboard/");
                } else {
                    res.set_header("Location", "/profile/?registered=1");
                }
                res.status = 302;
            } catch (const std::exception& e) {
                std::string error = "Ошибка при регистрации: " + std::string(e.what());
                res.set_content(HtmlGenerator::registration_form(error, user), "text/html; charset=utf-8");
            }
        });

        // Панель организации
        svr.Get("/organization/dashboard/", [&db](const Request& req, Response& res) {
            int64_t user_id = get_user_id_from_session(req);
            if (user_id == 0) {
                res.set_header("Location", "/login/");
                res.status = 302;
                return;
            }
            
            User user = db.get_user(user_id);
            if (user.user_id == 0 || !user.is_organization()) {
                res.set_content(HtmlGenerator::base_template("Ошибка", "<div class='alert alert-danger'>Организация не найдена</div>", "", &user), "text/html; charset=utf-8");
                return;
            }
            res.set_content(HtmlGenerator::organization_dashboard(db, user_id, &user), "text/html; charset=utf-8");
        });

        // Создание отеля (GET)
        svr.Get("/hotels/create/", [&db](const Request& req, Response& res) {
            int64_t user_id = get_user_id_from_session(req);
            if (user_id == 0) {
                res.set_header("Location", "/login/");
                res.status = 302;
                return;
            }
            
            User user = db.get_user(user_id);
            if (user.user_id == 0 || !user.is_organization()) {
                res.set_content(HtmlGenerator::base_template("Ошибка", "<div class='alert alert-danger'>Организация не найдена</div>", "", &user), "text/html; charset=utf-8");
                return;
            }
            res.set_content(HtmlGenerator::hotel_form(user_id, "", Hotel(), &user), "text/html; charset=utf-8");
        });

        // Создание отеля (POST)
        svr.Post("/hotels/create/", [&db](const Request& req, Response& res) {
            int64_t user_id = get_user_id_from_session(req);
            if (user_id == 0) {
                res.set_header("Location", "/login/");
                res.status = 302;
                return;
            }
            
            User user = db.get_user(user_id);
            if (user.user_id == 0 || !user.is_organization()) {
                res.set_content(HtmlGenerator::base_template("Ошибка", "<div class='alert alert-danger'>Организация не найдена</div>", "", &user), "text/html; charset=utf-8");
                return;
            }
            
            auto params = parse_form_data(req.body);
            
            Hotel hotel;
            hotel.organization_id = user_id;
            hotel.name = params.count("name") ? params["name"] : "";
            hotel.description = params.count("description") ? params["description"] : "";
            hotel.address = params.count("address") ? params["address"] : "";
            
            if (hotel.name.empty()) {
                std::string error = "Укажите название отеля";
                res.set_content(HtmlGenerator::hotel_form(user_id, error, hotel, &user), "text/html; charset=utf-8");
                return;
            }
            
            try {
                db.create_hotel(hotel);
                res.set_header("Location", "/organization/dashboard/");
                res.status = 302;
            } catch (const std::exception& e) {
                std::string error = "Ошибка при создании отеля: " + std::string(e.what());
                res.set_content(HtmlGenerator::hotel_form(user_id, error, hotel, &user), "text/html; charset=utf-8");
            }
        });

        // Создание номера в отеле (GET)
        svr.Get(R"(/hotels/(\d+)/rooms/create/)", [&db](const Request& req, Response& res) {
            int64_t user_id = get_user_id_from_session(req);
            if (user_id == 0) {
                res.set_header("Location", "/login/");
                res.status = 302;
                return;
            }
            
            int64_t hotel_id = std::stoll(req.matches[1]);
            Hotel hotel = db.get_hotel(hotel_id);
            if (hotel.hotel_id == 0) {
                User user = db.get_user(user_id);
                res.set_content(HtmlGenerator::base_template("Ошибка", "<div class='alert alert-danger'>Отель не найден</div>", "", &user), "text/html; charset=utf-8");
                return;
            }
            
            if (hotel.organization_id != user_id) {
                User user = db.get_user(user_id);
                res.set_content(HtmlGenerator::base_template("Ошибка", "<div class='alert alert-danger'>У вас нет доступа к этому отелю</div>", "", &user), "text/html; charset=utf-8");
                return;
            }
            
            User user = db.get_user(user_id);
            res.set_content(HtmlGenerator::room_form_for_hotel(db, hotel_id, hotel.organization_id, "", Room(), &user), "text/html; charset=utf-8");
        });

        // Создание номера в отеле (POST)
        svr.Post(R"(/hotels/(\d+)/rooms/create/)", [&db](const Request& req, Response& res) {
            int64_t user_id = get_user_id_from_session(req);
            if (user_id == 0) {
                res.set_header("Location", "/login/");
                res.status = 302;
                return;
            }
            
            int64_t hotel_id = std::stoll(req.matches[1]);
            Hotel hotel = db.get_hotel(hotel_id);
            if (hotel.hotel_id == 0) {
                User user = db.get_user(user_id);
                res.set_content(HtmlGenerator::base_template("Ошибка", "<div class='alert alert-danger'>Отель не найден</div>", "", &user), "text/html; charset=utf-8");
                return;
            }
            
            if (hotel.organization_id != user_id) {
                User user = db.get_user(user_id);
                res.set_content(HtmlGenerator::base_template("Ошибка", "<div class='alert alert-danger'>У вас нет доступа к этому отелю</div>", "", &user), "text/html; charset=utf-8");
                return;
            }
            
            auto params = parse_form_data(req.body);
            
            Room room;
            room.hotel_id = hotel_id;
            room.number = params.count("number") ? params["number"] : "";
            room.name = params.count("name") ? params["name"] : "";
            room.description = params.count("description") ? params["description"] : "";
            room.type_name = params.count("type_name") ? params["type_name"] : "";
            
            std::string price_str = params.count("price_per_day") ? params["price_per_day"] : "0";
            try {
                room.price_per_day = std::stod(price_str);
                if (room.price_per_day < 0) {
                    room.price_per_day = 0;
                }
            } catch (...) {
                room.price_per_day = 0;
            }
            
            if (room.number.empty() || room.name.empty() || room.type_name.empty() || room.price_per_day <= 0) {
                std::string error = "Заполните все обязательные поля (включая цену за день)";
                User user = db.get_user(user_id);
                res.set_content(HtmlGenerator::room_form_for_hotel(db, hotel_id, hotel.organization_id, error, room, &user), "text/html; charset=utf-8");
                return;
            }
            
            try {
                db.create_room(room);
                res.set_header("Location", "/organization/dashboard/");
                res.status = 302;
            } catch (const std::exception& e) {
                std::string error = "Ошибка при создании номера: " + std::string(e.what());
                User user = db.get_user(user_id);
                res.set_content(HtmlGenerator::room_form_for_hotel(db, hotel_id, hotel.organization_id, error, room, &user), "text/html; charset=utf-8");
            }
        });

        // Вход (GET)
        svr.Get("/login/", [&db](const Request& req, Response& res) {
            int64_t user_id = get_user_id_from_session(req);
            if (user_id != 0) {
                res.set_header("Location", "/profile/");
                res.status = 302;
                return;
            }
            res.set_content(HtmlGenerator::login_form(), "text/html; charset=utf-8");
        });

        // Вход (POST)
        svr.Post("/login/", [&db](const Request& req, Response& res) {
            auto params = parse_form_data(req.body);
            
            std::string email = params.count("email") ? params["email"] : "";
            std::string password = params.count("password") ? params["password"] : "";
            
            if (email.empty() || password.empty()) {
                std::string error = "Заполните все поля";
                res.set_content(HtmlGenerator::login_form(error), "text/html; charset=utf-8");
                return;
            }
            
            User user = db.get_user_by_email(email);
            if (user.user_id == 0) {
                std::string error = "Неверный email или пароль";
                res.set_content(HtmlGenerator::login_form(error), "text/html; charset=utf-8");
                return;
            }
            
            if (user.password != password) {
                std::string error = "Неверный email или пароль";
                res.set_content(HtmlGenerator::login_form(error), "text/html; charset=utf-8");
                return;
            }
            
            set_user_session(res, user.user_id);
            res.set_header("Location", "/profile/");
            res.status = 302;
        });

        // Выход
        svr.Get("/logout/", [&db](const Request& req, Response& res) {
            clear_user_session(res);
            res.set_header("Location", "/");
            res.status = 302;
        });

        // Профиль (GET)
        svr.Get("/profile/", [&db](const Request& req, Response& res) {
            int64_t user_id = get_user_id_from_session(req);
            if (user_id == 0) {
                res.set_header("Location", "/login/");
                res.status = 302;
                return;
            }
            
            User user = db.get_user(user_id);
            if (user.user_id == 0) {
                clear_user_session(res);
                res.set_header("Location", "/login/");
                res.status = 302;
                return;
            }
            
            std::string success = "";
            if (req.has_param("registered")) {
                success = "Регистрация успешна! Добро пожаловать!";
            } else if (req.has_param("updated")) {
                success = "Данные успешно обновлены!";
            } else if (req.has_param("password_updated")) {
                success = "Пароль успешно изменен!";
            }
            
            res.set_content(HtmlGenerator::profile_page(user, "", success), "text/html; charset=utf-8");
        });

        // Профиль (POST) - обновление данных
        svr.Post("/profile/", [&db](const Request& req, Response& res) {
            int64_t user_id = get_user_id_from_session(req);
            if (user_id == 0) {
                res.set_header("Location", "/login/");
                res.status = 302;
                return;
            }
            
            User user = db.get_user(user_id);
            if (user.user_id == 0) {
                clear_user_session(res);
                res.set_header("Location", "/login/");
                res.status = 302;
                return;
            }
            
            auto params = parse_form_data(req.body);
            
            user.full_name = params.count("full_name") ? params["full_name"] : "";
            user.phone = params.count("phone") ? params["phone"] : "";
            std::string new_email = params.count("email") ? params["email"] : "";
            
            if (user.full_name.empty() || user.phone.empty() || new_email.empty()) {
                std::string error = "Заполните все обязательные поля";
                res.set_content(HtmlGenerator::profile_page(user, error), "text/html; charset=utf-8");
                return;
            }
            
            // Проверка, не занят ли новый email другим пользователем
            if (new_email != user.email) {
                User existing = db.get_user_by_email(new_email);
                if (existing.user_id != 0 && existing.user_id != user.user_id) {
                    std::string error = "Пользователь с таким email уже существует";
                    res.set_content(HtmlGenerator::profile_page(user, error), "text/html; charset=utf-8");
                    return;
                }
            }
            
            user.email = new_email;
            
            if (user.is_organization()) {
                user.organization_name = params.count("organization_name") ? params["organization_name"] : "";
                if (user.organization_name.empty()) {
                    std::string error = "Укажите название организации";
                    res.set_content(HtmlGenerator::profile_page(user, error), "text/html; charset=utf-8");
                    return;
                }
            }
            
            try {
                db.update_user(user);
                res.set_header("Location", "/profile/?updated=1");
                res.status = 302;
            } catch (const std::exception& e) {
                std::string error = "Ошибка при обновлении данных: " + std::string(e.what());
                res.set_content(HtmlGenerator::profile_page(user, error), "text/html; charset=utf-8");
            }
        });

        // Изменение пароля (POST)
        svr.Post("/profile/password/", [&db](const Request& req, Response& res) {
            int64_t user_id = get_user_id_from_session(req);
            if (user_id == 0) {
                res.set_header("Location", "/login/");
                res.status = 302;
                return;
            }
            
            User user = db.get_user(user_id);
            if (user.user_id == 0) {
                clear_user_session(res);
                res.set_header("Location", "/login/");
                res.status = 302;
                return;
            }
            
            auto params = parse_form_data(req.body);
            
            std::string current_password = params.count("current_password") ? params["current_password"] : "";
            std::string new_password = params.count("new_password") ? params["new_password"] : "";
            std::string new_password_confirm = params.count("new_password_confirm") ? params["new_password_confirm"] : "";
            
            if (current_password.empty() || new_password.empty() || new_password_confirm.empty()) {
                std::string error = "Заполните все поля";
                res.set_content(HtmlGenerator::profile_page(user, error), "text/html; charset=utf-8");
                return;
            }
            
            if (user.password != current_password) {
                std::string error = "Текущий пароль неверен";
                res.set_content(HtmlGenerator::profile_page(user, error), "text/html; charset=utf-8");
                return;
            }
            
            if (new_password != new_password_confirm) {
                std::string error = "Новые пароли не совпадают";
                res.set_content(HtmlGenerator::profile_page(user, error), "text/html; charset=utf-8");
                return;
            }
            
            try {
                db.update_user_password(user_id, new_password);
                res.set_header("Location", "/profile/?password_updated=1");
                res.status = 302;
            } catch (const std::exception& e) {
                std::string error = "Ошибка при изменении пароля: " + std::string(e.what());
                res.set_content(HtmlGenerator::profile_page(user, error), "text/html; charset=utf-8");
            }
        });

        // Контакты
        svr.Get("/contact/", [&db, &get_user_from_session](const Request& req, Response& res) {
            User user = get_user_from_session(req);
            res.set_content(HtmlGenerator::contact_page(&user), "text/html; charset=utf-8");
        });

        std::cout << "Сервер запущен на http://localhost:8080" << std::endl;
        std::cout << "Нажмите Ctrl+C для остановки" << std::endl;
        
        svr.listen("0.0.0.0", 8080);
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

