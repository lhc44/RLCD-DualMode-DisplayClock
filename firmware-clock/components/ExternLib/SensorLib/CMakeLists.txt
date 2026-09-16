set(src_dirs ./src
             ./src/touch
             ./src/platform
             ./src/bosch
             ./src/bosch/BMM150
             ./src/bosch/common
             )
set(include_dirs ./src
                ./src/REG
                ./src/touch
                ./src/platform
                ./src/bosch
                ./src/bosch/firmware
                ./src/bosch/BMM150
                ./src/bosch/common)

idf_component_register(SRC_DIRS ${src_dirs} 
                        INCLUDE_DIRS ${include_dirs}
                        # ESP-IDF 6 owns driver/i2c_master.h in this component;
                        # declare it explicitly instead of relying on transitive headers.
                        REQUIRES esp_timer esp_driver_gpio esp_driver_i2c esp_driver_spi driver)
