#include "../include/models.h"
#include "../include/database.h"
#include "../include/html_generator.h"
#include <httplib.h>
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <ctime>

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

int main() {
    try {
        Database db("hotels.db");
        Server svr;

        // Главная страница
        svr.Get("/", [&db](const Request& req, Response& res) {
            res.set_content(HtmlGenerator::home_page(db), "text/html; charset=utf-8");
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

                std::string price_str = params.count("total_price") ? params["total_price"] : "0";
                booking.total_price = std::stod(price_str);

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

        // Контакты
        svr.Get("/contact/", [&db](const Request& req, Response& res) {
            res.set_content(HtmlGenerator::contact_page(), "text/html; charset=utf-8");
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

