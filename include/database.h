#ifndef DATABASE_H
#define DATABASE_H

#include "models.h"
#include <sqlite3.h>
#include <vector>
#include <memory>
#include <stdexcept>
#include <sstream>

class Database {
private:
    sqlite3* db;
    std::string db_path;

    void execute(const std::string& sql) {
        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::string error = errMsg ? errMsg : "Unknown error";
            sqlite3_free(errMsg);
            throw std::runtime_error("Database error: " + error);
        }
    }

public:
    Database(const std::string& path = "hotels.db") : db_path(path), db(nullptr) {
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(db)));
        }
        initialize();
    }

    ~Database() {
        if (db) {
            sqlite3_close(db);
        }
    }

    void initialize() {
        // Создание таблиц
        execute(R"(
            CREATE TABLE IF NOT EXISTS rooms (
                room_id INTEGER PRIMARY KEY AUTOINCREMENT,
                number TEXT UNIQUE NOT NULL,
                name TEXT NOT NULL,
                description TEXT,
                type_name TEXT NOT NULL,
                created_at TEXT NOT NULL,
                updated_at TEXT NOT NULL
            )
        )");

        execute(R"(
            CREATE TABLE IF NOT EXISTS guests (
                guest_id INTEGER PRIMARY KEY AUTOINCREMENT,
                first_name TEXT NOT NULL,
                last_name TEXT NOT NULL,
                middle_name TEXT,
                passport_number TEXT UNIQUE NOT NULL,
                email TEXT,
                phone TEXT NOT NULL,
                created_at TEXT NOT NULL,
                updated_at TEXT NOT NULL
            )
        )");

        execute(R"(
            CREATE TABLE IF NOT EXISTS bookings (
                booking_id INTEGER PRIMARY KEY AUTOINCREMENT,
                guest_id INTEGER NOT NULL,
                room_id INTEGER NOT NULL,
                check_in_date TEXT NOT NULL,
                check_out_date TEXT NOT NULL,
                adults_count INTEGER NOT NULL DEFAULT 1,
                children_count INTEGER NOT NULL DEFAULT 0,
                total_price REAL NOT NULL,
                special_requests TEXT,
                created_at TEXT NOT NULL,
                updated_at TEXT NOT NULL,
                FOREIGN KEY (guest_id) REFERENCES guests(guest_id),
                FOREIGN KEY (room_id) REFERENCES rooms(room_id)
            )
        )");
    }

    // Room operations
    std::vector<Room> get_all_rooms(const std::string& type_filter = "") {
        std::vector<Room> rooms;
        std::string sql = "SELECT room_id, number, name, description, type_name, created_at, updated_at FROM rooms";
        if (!type_filter.empty()) {
            sql += " WHERE type_name LIKE '%" + type_filter + "%'";
        }
        sql += " ORDER BY number";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                Room room;
                room.room_id = sqlite3_column_int64(stmt, 0);
                room.number = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                room.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                room.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                room.type_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                room.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
                room.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
                rooms.push_back(room);
            }
        }
        sqlite3_finalize(stmt);
        return rooms;
    }

    Room get_room(int64_t id) {
        std::string sql = "SELECT room_id, number, name, description, type_name, created_at, updated_at FROM rooms WHERE room_id = ?";
        sqlite3_stmt* stmt;
        Room room;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, id);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                room.room_id = sqlite3_column_int64(stmt, 0);
                room.number = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                room.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                room.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                room.type_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                room.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
                room.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            }
        }
        sqlite3_finalize(stmt);
        return room;
    }

    std::vector<std::string> get_room_types() {
        std::vector<std::string> types;
        std::string sql = "SELECT DISTINCT type_name FROM rooms";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                types.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            }
        }
        sqlite3_finalize(stmt);
        return types;
    }

    // Guest operations
    std::vector<Guest> get_all_guests(const std::string& search = "") {
        std::vector<Guest> guests;
        std::string sql = "SELECT guest_id, first_name, last_name, middle_name, passport_number, email, phone, created_at, updated_at FROM guests";
        
        if (!search.empty()) {
            sql += " WHERE first_name LIKE '%" + search + "%' OR last_name LIKE '%" + search + "%' OR phone LIKE '%" + search + "%' OR email LIKE '%" + search + "%'";
        }
        sql += " ORDER BY last_name, first_name";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                Guest guest;
                guest.guest_id = sqlite3_column_int64(stmt, 0);
                guest.first_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                guest.last_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                const char* middle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                guest.middle_name = middle ? middle : "";
                guest.passport_number = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                const char* email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
                guest.email = email ? email : "";
                guest.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
                guest.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
                guest.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
                guests.push_back(guest);
            }
        }
        sqlite3_finalize(stmt);
        return guests;
    }

    Guest get_guest(int64_t id) {
        std::string sql = "SELECT guest_id, first_name, last_name, middle_name, passport_number, email, phone, created_at, updated_at FROM guests WHERE guest_id = ?";
        sqlite3_stmt* stmt;
        Guest guest;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, id);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                guest.guest_id = sqlite3_column_int64(stmt, 0);
                guest.first_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                guest.last_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                const char* middle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                guest.middle_name = middle ? middle : "";
                guest.passport_number = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                const char* email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
                guest.email = email ? email : "";
                guest.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
                guest.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
                guest.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            }
        }
        sqlite3_finalize(stmt);
        return guest;
    }

    int64_t create_guest(const Guest& guest) {
        std::string now = get_current_datetime();
        std::string sql = "INSERT INTO guests (first_name, last_name, middle_name, passport_number, email, phone, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, guest.first_name.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, guest.last_name.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, guest.middle_name.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, guest.passport_number.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 5, guest.email.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 6, guest.phone.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 7, now.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 8, now.c_str(), -1, SQLITE_STATIC);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                throw std::runtime_error("Failed to create guest");
            }
        }
        int64_t id = sqlite3_last_insert_rowid(db);
        sqlite3_finalize(stmt);
        return id;
    }

    // Booking operations
    std::vector<Booking> get_all_bookings(const std::string& search = "") {
        std::vector<Booking> bookings;
        std::string sql = R"(
            SELECT b.booking_id, b.guest_id, b.room_id, b.check_in_date, b.check_out_date, 
                   b.adults_count, b.children_count, b.total_price, b.special_requests, 
                   b.created_at, b.updated_at
            FROM bookings b
        )";
        
        if (!search.empty()) {
            sql += " JOIN guests g ON b.guest_id = g.guest_id JOIN rooms r ON b.room_id = r.room_id";
            sql += " WHERE g.first_name LIKE '%" + search + "%' OR g.last_name LIKE '%" + search + "%' OR r.number LIKE '%" + search + "%' OR r.name LIKE '%" + search + "%'";
        }
        sql += " ORDER BY b.check_in_date DESC";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                Booking booking;
                booking.booking_id = sqlite3_column_int64(stmt, 0);
                booking.guest_id = sqlite3_column_int64(stmt, 1);
                booking.room_id = sqlite3_column_int64(stmt, 2);
                booking.check_in_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                booking.check_out_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                booking.adults_count = sqlite3_column_int(stmt, 5);
                booking.children_count = sqlite3_column_int(stmt, 6);
                booking.total_price = sqlite3_column_double(stmt, 7);
                const char* requests = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
                booking.special_requests = requests ? requests : "";
                booking.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
                booking.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
                bookings.push_back(booking);
            }
        }
        sqlite3_finalize(stmt);
        return bookings;
    }

    Booking get_booking(int64_t id) {
        std::string sql = "SELECT booking_id, guest_id, room_id, check_in_date, check_out_date, adults_count, children_count, total_price, special_requests, created_at, updated_at FROM bookings WHERE booking_id = ?";
        sqlite3_stmt* stmt;
        Booking booking;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, id);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                booking.booking_id = sqlite3_column_int64(stmt, 0);
                booking.guest_id = sqlite3_column_int64(stmt, 1);
                booking.room_id = sqlite3_column_int64(stmt, 2);
                booking.check_in_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                booking.check_out_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                booking.adults_count = sqlite3_column_int(stmt, 5);
                booking.children_count = sqlite3_column_int(stmt, 6);
                booking.total_price = sqlite3_column_double(stmt, 7);
                const char* requests = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
                booking.special_requests = requests ? requests : "";
                booking.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
                booking.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
            }
        }
        sqlite3_finalize(stmt);
        return booking;
    }

    std::vector<Booking> get_guest_bookings(int64_t guest_id) {
        std::vector<Booking> bookings;
        std::string sql = "SELECT booking_id, guest_id, room_id, check_in_date, check_out_date, adults_count, children_count, total_price, special_requests, created_at, updated_at FROM bookings WHERE guest_id = ? ORDER BY check_in_date DESC";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, guest_id);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                Booking booking;
                booking.booking_id = sqlite3_column_int64(stmt, 0);
                booking.guest_id = sqlite3_column_int64(stmt, 1);
                booking.room_id = sqlite3_column_int64(stmt, 2);
                booking.check_in_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                booking.check_out_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                booking.adults_count = sqlite3_column_int(stmt, 5);
                booking.children_count = sqlite3_column_int(stmt, 6);
                booking.total_price = sqlite3_column_double(stmt, 7);
                const char* requests = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
                booking.special_requests = requests ? requests : "";
                booking.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
                booking.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
                bookings.push_back(booking);
            }
        }
        sqlite3_finalize(stmt);
        return bookings;
    }

    bool is_room_available(int64_t room_id, const std::string& check_in, const std::string& check_out) {
        std::string sql = "SELECT COUNT(*) FROM bookings WHERE room_id = ? AND check_in_date < ? AND check_out_date > ?";
        sqlite3_stmt* stmt;
        bool available = true;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, room_id);
            sqlite3_bind_text(stmt, 2, check_out.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, check_in.c_str(), -1, SQLITE_STATIC);
            
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int count = sqlite3_column_int(stmt, 0);
                available = (count == 0);
            }
        }
        sqlite3_finalize(stmt);
        return available;
    }

    int64_t create_booking(const Booking& booking) {
        std::string now = get_current_datetime();
        std::string sql = "INSERT INTO bookings (guest_id, room_id, check_in_date, check_out_date, adults_count, children_count, total_price, special_requests, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, booking.guest_id);
            sqlite3_bind_int64(stmt, 2, booking.room_id);
            sqlite3_bind_text(stmt, 3, booking.check_in_date.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, booking.check_out_date.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 5, booking.adults_count);
            sqlite3_bind_int(stmt, 6, booking.children_count);
            sqlite3_bind_double(stmt, 7, booking.total_price);
            sqlite3_bind_text(stmt, 8, booking.special_requests.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 9, now.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 10, now.c_str(), -1, SQLITE_STATIC);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                throw std::runtime_error("Failed to create booking");
            }
        }
        int64_t id = sqlite3_last_insert_rowid(db);
        sqlite3_finalize(stmt);
        return id;
    }

    int get_rooms_count() {
        std::string sql = "SELECT COUNT(*) FROM rooms";
        sqlite3_stmt* stmt;
        int count = 0;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                count = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);
        return count;
    }

    int get_guests_count() {
        std::string sql = "SELECT COUNT(*) FROM guests";
        sqlite3_stmt* stmt;
        int count = 0;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                count = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);
        return count;
    }

    int get_bookings_count() {
        std::string sql = "SELECT COUNT(*) FROM bookings";
        sqlite3_stmt* stmt;
        int count = 0;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                count = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);
        return count;
    }
};

#endif // DATABASE_H

