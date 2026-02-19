#include "imu_service.h"
#include "lumari_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
#include "input_hal.h"
#include "qmi8658.h"
#endif

static const char *TAG = "imu_service";

static bool s_imu_ok = false;

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
static qmi8658_dev_t s_qmi;
static float s_last_ax = 0, s_last_ay = 0, s_last_az = 0;
static int64_t s_last_shake_ms = 0;
#define SHAKE_MAGNITUDE_THRESHOLD  18.0f
#define SHAKE_DEBOUNCE_MS          500
#endif

void imu_service_init(void)
{
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    i2c_master_bus_handle_t bus = input_hal_get_i2c_bus();
    if (bus == nullptr) {
        ESP_LOGW(TAG, "No I2C bus for IMU");
        return;
    }
    esp_err_t err = qmi8658_init(&s_qmi, bus, QMI8658_ADDRESS_HIGH);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "QMI8658 init failed (%s)", esp_err_to_name(err));
        return;
    }
    qmi8658_set_accel_range(&s_qmi, QMI8658_ACCEL_RANGE_8G);
    qmi8658_set_accel_odr(&s_qmi, QMI8658_ACCEL_ODR_1000HZ);
    qmi8658_set_gyro_range(&s_qmi, QMI8658_GYRO_RANGE_512DPS);
    qmi8658_set_gyro_odr(&s_qmi, QMI8658_GYRO_ODR_1000HZ);
    qmi8658_set_accel_unit_mps2(&s_qmi, true);
    s_imu_ok = true;
    ESP_LOGI(TAG, "QMI8658 IMU init OK");
#else
    s_imu_ok = false;
#endif
}

void imu_service_read_accel(int16_t *ax, int16_t *ay, int16_t *az)
{
    if (ax) *ax = 0;
    if (ay) *ay = 0;
    if (az) *az = 0;

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    if (!s_imu_ok) return;
    qmi8658_data_t data;
    bool ready = false;
    if (qmi8658_is_data_ready(&s_qmi, &ready) != ESP_OK || !ready) return;
    if (qmi8658_read_sensor_data(&s_qmi, &data) != ESP_OK) return;
    /* Report in 0.01 m/s² units (e.g. 981 = 9.81 m/s²) */
    if (ax) *ax = (int16_t)(data.accelX * 100);
    if (ay) *ay = (int16_t)(data.accelY * 100);
    if (az) *az = (int16_t)(data.accelZ * 100);
#endif
}

bool imu_service_shake_detected(void)
{
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    if (!s_imu_ok) return false;
    qmi8658_data_t data;
    bool ready = false;
    if (qmi8658_is_data_ready(&s_qmi, &ready) != ESP_OK || !ready) return false;
    if (qmi8658_read_sensor_data(&s_qmi, &data) != ESP_OK) return false;

    float ax = (float)data.accelX, ay = (float)data.accelY, az = (float)data.accelZ;
    float mag = sqrtf(ax * ax + ay * ay + az * az);
    int64_t now = esp_timer_get_time() / 1000;
    if (mag >= SHAKE_MAGNITUDE_THRESHOLD && (now - s_last_shake_ms) >= SHAKE_DEBOUNCE_MS) {
        s_last_shake_ms = now;
        return true;
    }
    return false;
#else
    return false;
#endif
}
