/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "bsp.h"
#include "app_usb.h"
#include "usb_descriptors.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "app_lcd.h"
#include "log.h"


static const char *TAG = "ud";

// GPIO18 is the board's dedicated KEY.  The display application returns to
// the untouched clock application in OTA slot 0 after a deliberate long hold.
static void return_to_clock_task(void *arg)
{
    (void)arg;
    const gpio_config_t key = {
        .pin_bit_mask = 1ULL << GPIO_NUM_18,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    (void)gpio_config(&key);
    TickType_t pressed_since = 0;
    for (;;) {
        const TickType_t now = xTaskGetTickCount();
        if (gpio_get_level(GPIO_NUM_18) == 0) {
            if (pressed_since == 0) {
                pressed_since = now;
            } else if (now - pressed_since >= pdMS_TO_TICKS(1500)) {
                const esp_partition_t *clock_slot = esp_partition_find_first(
                    ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
                if (clock_slot && esp_ota_set_boot_partition(clock_slot) == ESP_OK) {
                    vTaskDelay(pdMS_TO_TICKS(30));
                    esp_restart();
                }
                pressed_since = now;
            }
        } else {
            pressed_since = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "pcj");
		LOGI( "%s %d %d/%d",__func__,__LINE__,heap_caps_get_free_size(MALLOC_CAP_INTERNAL),heap_caps_get_total_size(MALLOC_CAP_INTERNAL));
	  
    app_usb_init();
		LOGI( "%s %d %d/%d",__func__,__LINE__,heap_caps_get_free_size(MALLOC_CAP_INTERNAL),heap_caps_get_total_size(MALLOC_CAP_INTERNAL));
	  
    app_lcd_init();
    xTaskCreatePinnedToCore(return_to_clock_task, "return_clock", 2048,
                            NULL, 5, NULL, 1);

    /* The ESP-SparkBot does not support touch functionality. 
     * To enable touch features, consider upgrading to a screen that supports touch input.
     */
    // app_touch_init();
}
