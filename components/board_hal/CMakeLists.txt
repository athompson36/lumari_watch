idf_component_register(
    SRCS "display_hal.cpp"
    INCLUDE_DIRS "."
    REQUIRES driver esp_lcd config esp_lcd_co5300 esp_lcd_qemu_rgb
)
