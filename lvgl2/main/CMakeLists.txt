idf_component_register(SRCS  "main.c" "clippits.c" "BasicImages.c"  "Screens/Screens.c" "StepperMotor" "Thermal.c"
                    PRIV_REQUIRES spi_flash
                    INCLUDE_DIRS
                    "."
                    "Screens"
                    )
