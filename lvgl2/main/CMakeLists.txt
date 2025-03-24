idf_component_register(SRCS  "main.c" 
                                "Screens/Screens.c"
                                "WiFi.c" 
                                "StepperMotor" 
                                "Thermal.c" 
                                "touch.c" 
                                "Screens/Styles.c" 
                                "Screens/SplashScreen.c"
                    PRIV_REQUIRES spi_flash
                    INCLUDE_DIRS
                    "."
                    "Screens"
                    REQUIRES esp_wifi
                    REQUIRES nvs_flash
                    REQUIRES esp-mqtt
                    REQUIRES json
                    )


                   