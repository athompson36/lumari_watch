# Optional unit tests (quest, inventory) when CONFIG_LUMARI_RUN_UNIT_TESTS=y
set(_main_srcs "main.cpp" "app_entry.cpp" "phase0_demo.cpp")
set(_main_reqs "config" "display_hal" "engine" "input" "services" "quest" "inventory" "aura" "cutscene" "storage" "esp_timer")
if(EXISTS "${CMAKE_SOURCE_DIR}/sdkconfig")
  file(STRINGS "${CMAKE_SOURCE_DIR}/sdkconfig" _cfg LIMIT_COUNT 1 REGEX "^CONFIG_LUMARI_RUN_UNIT_TESTS=y$")
  if(_cfg)
    list(APPEND _main_srcs "test_harness.c")
    list(APPEND _main_reqs "unity")
  endif()
endif()
idf_component_register(
    SRCS ${_main_srcs}
    INCLUDE_DIRS "."
    REQUIRES ${_main_reqs}
)
