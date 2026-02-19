idf_component_register(
    SRCS "main.cpp" "app_entry.cpp" "phase0_demo.cpp"
    INCLUDE_DIRS "."
    REQUIRES config display_hal engine input services quest esp_timer
)
