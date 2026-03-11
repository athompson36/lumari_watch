idf_component_register(
    SRCS "input_hal.cpp"
    INCLUDE_DIRS "."
    REQUIRES driver config freertos esp_lcd espressif__esp_lcd_touch_ft5x06
)
