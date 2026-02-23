#pragma once
#include <stdint.h>
#include <stdbool.h>

void imu_service_init(void);
/* Call once per frame; caches one IMU read for read_accel, shake, step. */
void imu_service_update(void);
void imu_service_read_accel(int16_t *ax, int16_t *ay, int16_t *az);
bool imu_service_shake_detected(void);
unsigned imu_service_get_step_delta(void);
