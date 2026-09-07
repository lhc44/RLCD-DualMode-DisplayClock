#include "usb_display_service.h"

#include "dual_mode_controller.h"
#include "display_bsp.h"
#include "usb_display_device_identity.h"
#include "usb_display_protocol.h"

#include <atomic>
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
// The Windows desktop transport arrives as 64-byte full-speed bulk packets.
// A full desktop frame is 15 KB of 64-byte full-speed bulk packets.  Use the
// known-good priority and the enlarged RX queue, but always block for one
// scheduler tick after servicing USB so CPU0's idle task, networking, and
// watchdog keep making forward progress.  The original UI/buttons remain on
// core 1.
constexpr UBaseType_t kUsbTaskPriority = 5;
constexpr BaseType_t kUsbTaskCore = 0;
constexpr size_t kReadBufferBytes = 512;
constexpr TickType_t kClockModeUsbPollTicks = pdMS_TO_TICKS(20);
constexpr char kTag[] = "usb_display";

DisplayPort *s_display = nullptr;
UsbDisplayMono1Receiver s_receiver;
uint8_t s_last_frame[kUsbDisplayMono1Bytes] = {};
std::atomic<bool> s_last_frame_valid{false};
std::atomic<bool> s_present_cached_frame_requested{false};
std::atomic<bool> s_panel_display_active{false};
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
            // The Windows sender elides duplicate desktop frames. Cache every
            // complete frame, including those received while Clock mode owns
            // the panel, so a later physical mode switch has an immediate
            // image to present.
            memcpy(s_last_frame, s_receiver.data(), s_receiver.size());
            s_last_frame_valid.store(true, std::memory_order_release);
            if (dual_mode_snapshot_load().mode == DualMode::Display && s_display) {
                (void)s_display->RLCD_PresentMono1(s_last_frame, sizeof(s_last_frame));
            }
            s_receiver.reset();
        }
    }
}

void present_cached_frame_if_requested()
{
    if (!s_present_cached_frame_requested.exchange(false, std::memory_order_acq_rel) ||
        !s_display || !s_last_frame_valid.load(std::memory_order_acquire) ||
        dual_mode_snapshot_load().mode != DualMode::Display) {
        return;
    }
    (void)s_display->RLCD_PresentMono1(s_last_frame, sizeof(s_last_frame));
}

void usb_task(void *)
{
    for (;;) {
        tud_task();
        present_cached_frame_if_requested();
        // There is no Windows frame producer while the device advertises the
        // clock PID.  Poll slowly in that mode so the original Wi-Fi/NTP and
        // weather tasks retain CPU0; switch to the short transport cadence as
        // soon as Display mode (or its pending reconnect) is selected.
        const bool display_transport_active =
            s_panel_display_active.load(std::memory_order_acquire);
        vTaskDelay(display_transport_active ? pdMS_TO_TICKS(1) : kClockModeUsbPollTicks);
    }
}

bool init_usb_phy()
{
    usb_phy_config_t config = {};
    config.controller = USB_PHY_CTRL_OTG;
    config.target = USB_PHY_TARGET_INT;
    config.otg_mode = USB_OTG_MODE_DEVICE;
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
    s_panel_display_active.store(dual_mode_snapshot_load().mode == DualMode::Display,
                                 std::memory_order_release);
    if (xTaskCreatePinnedToCore(usb_task, "usb_display", kUsbTaskStack, nullptr,
                                kUsbTaskPriority, nullptr, kUsbTaskCore) != pdPASS) {
        s_display = nullptr;
        return false;
    }
    return true;
}

bool usb_display_service_host_attached()
{
    return s_host_attached;
}

void usb_display_service_request_cached_frame()
{
    s_present_cached_frame_requested.store(true, std::memory_order_release);
}

void usb_display_service_set_display_active(bool active)
{
    s_panel_display_active.store(active, std::memory_order_release);
}

extern "C" bool usb_display_service_enumerate_as_display(void)
{
    return true;
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
extern "C" void tud_umount_cb(void)
{
    s_host_attached = false;
    s_receiver.reset();
    s_last_frame_valid.store(false, std::memory_order_release);
    s_present_cached_frame_requested.store(false, std::memory_order_release);
}
extern "C" void tud_suspend_cb(bool) { }
extern "C" void tud_resume_cb(void) { }
