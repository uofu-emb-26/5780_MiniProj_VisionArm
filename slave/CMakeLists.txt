# Add C source files here.
add_executable(project
    Src/slave.c
    Src/hal_gpio.c
    Src/stm32f0xx_hal_msp.c
    Src/stm32f0xx_it.c
)

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(project PROPERTIES ADDITIONAL_CLEAN_FILES project.map)

# Add user defined include paths
target_include_directories(project PRIVATE
    Inc
)

# Add project symbols (macros)
target_compile_definitions(project PRIVATE
)

# Add linked libraries
target_link_libraries(project
    STM32_Drivers
)

flash_target(project)
