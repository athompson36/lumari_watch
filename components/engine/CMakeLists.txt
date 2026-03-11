idf_component_register(
    SRCS
        "render_engine.cpp"
        "framebuffer_manager.cpp"
        "layer_manager.cpp"
        "sprite_renderer.cpp"
        "ui_manager.cpp"
        "screens/ui_home_screen.cpp"
        "screens/ui_quest_screen.cpp"
        "screens/ui_inventory_screen.cpp"
        "screens/ui_crafting_screen.cpp"
        "screens/ui_lore_screen.cpp"
        "screens/ui_system_screen.cpp"
    INCLUDE_DIRS "." "screens"
    REQUIRES display_hal config creature quest inventory aura cutscene services freertos
)
