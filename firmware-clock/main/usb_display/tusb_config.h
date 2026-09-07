#pragma once

#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_TUSB_MCU OPT_MCU_ESP32S3
#define CFG_TUSB_OS OPT_OS_FREERTOS
#define CFG_TUSB_OS_INC_PATH freertos/
#define CFG_TUSB_DEBUG 0
#define CFG_TUD_ENABLED 1
#define CFG_TUD_ENDPOINT0_SIZE 64
#define CFG_TUD_VENDOR 1
#define CFG_TUD_VENDOR_EPSIZE 64
// The clock firmware schedules at 250 Hz.  This absorbs several USB bulk
// service intervals while the USB task yields to CPU0 idle/network work.
#define CFG_TUD_VENDOR_RX_BUFSIZE 8192
#define CFG_TUD_VENDOR_TX_BUFSIZE 64
#define CFG_TUD_HID 0

#ifdef __cplusplus
}
#endif
