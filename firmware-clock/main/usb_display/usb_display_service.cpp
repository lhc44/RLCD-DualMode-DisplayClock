#include "usb_display_service.h"

#include "dual_mode_controller.h"
#include "display_bsp.h"
#include "usb_display_protocol.h"

#include <string.h>

extern "C" {
#include "device/usbd.h"
#include "esp_log.h"
#include "esp_private/usb_phy.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tusb.h"
}

namespace {
constexpr uint32_t kUsbTaskStack = 4096;
constexpr UBaseType_t kUsbTaskPriority = 5;
constexpr size_t kReadBufferBytes = 64;
constexpr char kTag[] = "usb_display";

DisplayPort *s_display = nullptr;
UsbDisplayMono1Receiver s_receiver;
volatile bool s_host_attached = false;

void consume_vendor_bytes(const uint8_t *data, size_t length)
{
    static uint8_t header_bytes[sizeof(UsbDisplayFrameHeader)] = {};
    static size_t header_received = 0;
    while (length > 0) {
        if (!s_receiver.active()) {
            const size_t take = (sizeof(header_bytes) - header_received) < length
                                    ? (sizeof(header_bytes) - header_received)
                                    : length;
            memcpy(header_bytes + header_received, data, take);
            header_received += take;
            data += take;
            length -= take;
            if (header_received < sizeof(header_bytes)) {
                return;
            }
            UsbDisplayFrameHeader header = {};
            memcpy(&header, header_bytes, sizeof(header));
            header_received = 0;
            if (!s_receiver.begin(header)) {
                ESP_LOGW(kTag, "drop unsupported frame id=%lu", (unsigned long)usb_display_frame_id(header));
                continue;
            }
        }

        // A USB bulk read can straddle the final payload byte and the next
        // header. Consume only this frame's remaining bytes, then let the
        // loop parse any following header from the same read.
        const size_t take = s_receiver.remaining() < length
                                ? s_receiver.remaining()
                                : length;
        if (take == 0 || !s_receiver.append(data, take)) {
            ESP_LOGW(kTag, "drop malformed Mono1 payload");
            s_receiver.reset();
            return;
        }
        data += take;
        length -= take;
        if (s_receiver.complete()) {
            if (dual_mode_snapshot_load().mode == DualMode::Display && s_display) {
                (void)s_display->RLCD_PresentMono1(s_receiver.data(), s_receiver.size());
            }
            s_receiver.reset();
        }
    }
}

void usb_task(void *)
{
    for (;;) {
        tud_task();
    }
}

bool init_usb_phy()
{
    usb_phy_config_t config = {
        .controller = USB_PHY_CTRL_OTG,
        .target = USB_PHY_TARGET_INT,
        .otg_mode = USB_OTG_MODE_DEVICE,
    };
    usb_phy_handle_t handle = nullptr;
    return usb_new_phy(&config, &handle) == ESP_OK;
}
}

bool usb_display_service_init(DisplayPort &display)
{
    if (s_display) {
        return s_display == &display;
    }
    if (!init_usb_phy() || !tusb_init()) {
        ESP_LOGE(kTag, "USB device initialization failed");
        return false;
    }
    s_display = &display;
    if (xTaskCreate(usb_task, "usb_display", kUsbTaskStack, nullptr,
                    kUsbTaskPriority, nullptr) != pdPASS) {
        s_display = nullptr;
        return false;
    }
    return true;
}

bool usb_display_service_host_attached()
{
    return s_host_attached;
}

extern "C" void tud_vendor_rx_cb(uint8_t interface_number)
{
    uint8_t buffer[kReadBufferBytes];
    while (tud_vendor_n_available(interface_number)) {
        const uint32_t read = tud_vendor_n_read(interface_number, buffer, sizeof(buffer));
        if (read == 0) {
            break;
        }
        consume_vendor_bytes(buffer, read);
    }
}

extern "C" void tud_mount_cb(void) { s_host_attached = true; }
extern "C" void tud_umount_cb(void) { s_host_attached = false; s_receiver.reset(); }
extern "C" void tud_suspend_cb(bool) { }
extern "C" void tud_resume_cb(void) { }
