#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} time_service_datetime_t;

void time_service_init(void);
bool time_service_get(time_service_datetime_t *out);
bool time_service_set(const time_service_datetime_t *dt);
bool time_service_ok(void);
