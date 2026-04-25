# Add C source files here.
add_executable(slave
    Src/slave.c
    Src/hal_gpio.c
    Src/stm32f0xx_hal_msp.c
    Src/stm32f0xx_it.c
    ../master/Src/motor.c
)

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(slave PROPERTIES ADDITIONAL_CLEAN_FILES slave.map)

# Add user defined include paths
target_include_directories(slave PRIVATE
    Inc
    ../master/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/SEGGER/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/Components/Common
)

# Add project symbols (macros)
target_compile_definitions(slave PRIVATE
)

# Add linked libraries
target_link_libraries(slave
    STM32_Drivers
    SEGGER_RTT
)

flash_target(slave)
