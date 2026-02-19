#pragma once
#include <stdint.h>
#include <stdbool.h>

void imu_service_init(void);
void imu_service_read_accel(int16_t *ax, int16_t *ay, int16_t *az);
bool imu_service_shake_detected(void);
