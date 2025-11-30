#include "datetime.h"

#include <stdbool.h>

// Функция проверки високосного года
static bool is_leap_year(uint16_t year) {
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

// Количество дней в месяцах (невисокосный год)
static const uint8_t days_in_month[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

// Количество дней в месяцах для високосного года
static const uint8_t days_in_month_leap[] = {
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static const uint32_t SECONDS_PER_MINUTE = 60;
static const uint32_t SECONDS_PER_HOUR = 3600;
static const uint32_t SECONDS_PER_DAY = 86400;

void datetime_time_from_timestamp(uint32_t timestamp, time_s *time)
{ 
    uint32_t time_remaining = timestamp % SECONDS_PER_DAY;
    time->hour = time_remaining / SECONDS_PER_HOUR;
    time_remaining %= SECONDS_PER_HOUR;
    time->minute = time_remaining / SECONDS_PER_MINUTE;
    time->second = time_remaining % SECONDS_PER_MINUTE;
}


// Прямая функция (из предыдущего ответа)
void datetime_from_timestamp(uint32_t timestamp, datetime_s *datetime)
{    
    uint16_t year = 2025;
    uint8_t month = 1;
    uint32_t days = timestamp / SECONDS_PER_DAY;
    
    // Вычисляем время
    uint32_t time_remaining = timestamp % SECONDS_PER_DAY;
    datetime->hour = time_remaining / SECONDS_PER_HOUR;
    time_remaining %= SECONDS_PER_HOUR;
    datetime->minute = time_remaining / SECONDS_PER_MINUTE;
    datetime->second = time_remaining % SECONDS_PER_MINUTE;
    
    // Обрабатываем дату
    while (days > 0) {
        bool leap = is_leap_year(year);
        const uint8_t *month_days = leap ? days_in_month_leap : days_in_month;
        uint8_t days_in_current_month = month_days[month - 1];
        
        if (days >= days_in_current_month) {
            days -= days_in_current_month;
            month++;
            if (month > 12) {
                month = 1;
                year++;
            }
        } else {
            break;
        }
    }
    
    datetime->year = year;
    datetime->month = month;
    datetime->day = days + 1; // +1 потому что начальный день = 1
}

// Обратная функция: преобразование даты/времени в timestamp
uint32_t datetime_to_timestamp(const datetime_s *datetime) 
{    
    // Базовая дата: 01.01.2025 00:00:00
    const uint16_t BASE_YEAR = 2025;
    
    // Проверка валидности входных данных
    if (datetime->year < BASE_YEAR || 
        datetime->month < 1 || datetime->month > 12 ||
        datetime->day < 1 || datetime->day > 31 ||
        datetime->hour > 23 || 
        datetime->minute > 59 || 
        datetime->second > 59) {
        return 0; // или другое значение ошибки
    }
    
    // Проверка корректности дня месяца
    bool leap = is_leap_year(datetime->year);
    const uint8_t *month_days = leap ? days_in_month_leap : days_in_month;
    if (datetime->day > month_days[datetime->month - 1]) {
        return 0; // Некорректная дата
    }
    
    uint32_t total_days = 0;
    
    // Вычисляем количество дней с BASE_YEAR до datetime->year
    for (uint16_t year = BASE_YEAR; year < datetime->year; year++) {
        total_days += is_leap_year(year) ? 366 : 365;
    }
    
    // Добавляем дни в текущем году с января до предыдущего месяца
    for (uint8_t m = 1; m < datetime->month; m++) {
        total_days += month_days[m - 1];
    }
    
    // Добавляем дни в текущем месяце (минус 1, т.к. день начинается с 1)
    total_days += (datetime->day - 1);
    
    // Вычисляем общее количество секунд
    uint32_t timestamp = total_days * SECONDS_PER_DAY;
    timestamp += datetime->hour * SECONDS_PER_HOUR;
    timestamp += datetime->minute * SECONDS_PER_MINUTE;
    timestamp += datetime->second;
    
    return timestamp;
}