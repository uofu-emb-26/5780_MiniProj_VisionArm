# Add C source files here.
add_executable(master
    Src/master.c
    Src/hal_gpio.c
    Src/stm32f0xx_hal_msp.c
    Src/stm32f0xx_it.c
    Src/i2c_config.c
    Src/motor.c
    Src/gyro.c
)

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(master PROPERTIES ADDITIONAL_CLEAN_FILES master.map)

# Add user defined include paths
target_include_directories(master PRIVATE
    Inc
)

# Add project symbols (macros)
target_compile_definitions(master PRIVATE
)

# Add linked libraries
target_link_libraries(master
    STM32_Drivers
    SEGGER_RTT
)

flash_target(master)
