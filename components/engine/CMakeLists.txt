idf_component_register(
    SRCS
        "render_engine.cpp"
        "framebuffer_manager.cpp"
        "layer_manager.cpp"
        "sprite_renderer.cpp"
    INCLUDE_DIRS "."
    REQUIRES display_hal config creature quest inventory aura cutscene freertos
)
