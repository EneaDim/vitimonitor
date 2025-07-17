# Custom Zephyr funtion that imports the mock_sensor/ subdirectory if the Kconfig
# option MOCK_SENSOR is defined
add_subdirectory_ifdef(CONFIG_MOCK_SENSOR mock_sensor)
