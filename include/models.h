#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>

struct Room {
    int64_t room_id = 0;
    int64_t hotel_id = 0;  // ID отеля, к которому относится номер
    std::string number;
    std::string name;
    std::string description;
    std::string type_name;
    double price_per_day = 0.0;  // Цена за день проживания
    std::string created_at;
    std::string updated_at;

    Room() = default;
    
    std::string to_string() const {
        return number + " - " + name;
    }
};

struct Guest {
    int64_t guest_id = 0;
    std::string first_name;
    std::string last_name;
    std::string middle_name;
    std::string passport_number;
    std::string email;
    std::string phone;
    std::string created_at;
    std::string updated_at;

    Guest() = default;
    
    std::string full_name() const {
        std::string result = last_name + " " + first_name;
        if (!middle_name.empty()) {
            result += " " + middle_name;
        }
        return result;
    }
    
    std::string to_string() const {
        return full_name();
    }
};

struct Booking {
    int64_t booking_id = 0;
    int64_t guest_id = 0;
    int64_t room_id = 0;
    std::string check_in_date;
    std::string check_out_date;
    int adults_count = 1;
    int children_count = 0;
    double total_price = 0.0;
    std::string special_requests;
    std::string created_at;
    std::string updated_at;

    Booking() = default;
    
    std::string to_string() const {
        return "Бронирование #" + std::to_string(booking_id);
    }
};

struct User {
    int64_t user_id = 0;
    std::string full_name;  // ФИО
    std::string phone;
    std::string email;
    std::string password;
    std::string user_type;  // "user" или "organization"
    std::string organization_name;  // только для организаций
    std::string created_at;
    std::string updated_at;

    User() = default;
    
    bool is_organization() const {
        return user_type == "organization";
    }
};

struct Hotel {
    int64_t hotel_id = 0;
    int64_t organization_id = 0;  // ID пользователя-организации
    std::string name;
    std::string description;
    std::string address;
    std::string created_at;
    std::string updated_at;

    Hotel() = default;
};

// Утилита для получения текущей даты/времени
inline std::string get_current_datetime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

inline std::string get_current_date() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d");
    return ss.str();
}

#endif // MODELS_H

