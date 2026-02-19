idf_component_register(
    SRCS "time_service.cpp" "imu_service.cpp"
    INCLUDE_DIRS "."
    REQUIRES config input qmi8658 esp_timer
)
