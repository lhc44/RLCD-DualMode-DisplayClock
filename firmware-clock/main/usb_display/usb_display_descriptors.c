#include <string.h>
#include "tusb.h"

enum { kVendorInterface = 0, kInterfaceCount = 1 };
enum { kVendorEndpoint = 1 };

static const tusb_desc_device_t kDeviceDescriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_UNSPECIFIED,
    .bDeviceSubClass = TUSB_CLASS_UNSPECIFIED,
    .bDeviceProtocol = TUSB_CLASS_UNSPECIFIED,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A,
    .idProduct = 0x2986,
    .bcdDevice = 0x0101,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

uint8_t const *tud_descriptor_device_cb(void)
{
    // Keep one stable PnP identity for the whole USB session.  Switching
    // ownership of the physical panel must not reset this board or ask
    // Windows to tear down and recreate the indirect display adapter.
    return (uint8_t const *)&kDeviceDescriptor;
}

#define USB_DISPLAY_CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_VENDOR_DESC_LEN)
static const uint8_t kConfigurationDescriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, kInterfaceCount, 0, USB_DISPLAY_CONFIG_TOTAL_LEN, 0, 100),
    TUD_VENDOR_DESCRIPTOR(kVendorInterface, 4, kVendorEndpoint,
                          0x80 | kVendorEndpoint, CFG_TUD_VENDOR_EPSIZE),
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return kConfigurationDescriptor;
}

static const char *const kStringDescriptors[] = {
    (const char[]){0x09, 0x04},
    "Espressif",
    "ESP32-S3-RLCD-4.2_R400x300_Emono1_Fps60_Bl16",
    "012-2021",
    "esp32s3udisp0_R400x300_Emono1_Fps60_Bl16",
};
static uint16_t s_string_descriptor[48];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    uint8_t count = 0;
    if (index == 0) {
        memcpy(&s_string_descriptor[1], kStringDescriptors[0], 2);
        count = 1;
    } else {
        if (index >= sizeof(kStringDescriptors) / sizeof(kStringDescriptors[0])) {
            return NULL;
        }
        const char *value = kStringDescriptors[index];
        count = (uint8_t)strlen(value);
        if (count > 47) {
            count = 47;
        }
        for (uint8_t i = 0; i < count; ++i) {
            s_string_descriptor[1 + i] = value[i];
        }
    }
    s_string_descriptor[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * count + 2));
    return s_string_descriptor;
}
