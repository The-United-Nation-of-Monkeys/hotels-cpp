#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>

struct Room {
    int64_t room_id = 0;
    std::string number;
    std::string name;
    std::string description;
    std::string type_name;
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

