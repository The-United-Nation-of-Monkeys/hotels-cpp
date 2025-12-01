cmake_minimum_required(VERSION 3.15)
project(HotelBooking CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Найти SQLite3
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# Скачать cpp-httplib
include(FetchContent)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(httplib)

# Исходные файлы
set(SOURCES
    src/main.cpp
)

# Заголовочные файлы
set(HEADERS
    include/models.h
    include/database.h
    include/html_generator.h
)

# Создать исполняемый файл
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Включить директории
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLITE3_INCLUDE_DIRS}
)

# Связать библиотеки
target_link_libraries(${PROJECT_NAME} PRIVATE
    httplib::httplib
    ${SQLITE3_LIBRARIES}
)

# Флаги компиляции
target_compile_options(${PROJECT_NAME} PRIVATE
    ${SQLITE3_CFLAGS_OTHER}
)

# Установка
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

