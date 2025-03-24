# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(
  SRCS "main-mqtt_led.c"
  # REQUIRES driver
  INCLUDE_DIRS "."
  # REQUIRES esp_driver_i2c
  PRIV_REQUIRES mqtt nvs_flash esp_netif
)