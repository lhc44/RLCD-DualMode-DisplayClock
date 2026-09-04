// USB Graphic protocol boundary for the Windows Mono1 secondary-display path.
#pragma once

#include <stddef.h>
#include <stdint.h>

inline constexpr uint16_t kUsbDisplayWidth = 400;
inline constexpr uint16_t kUsbDisplayHeight = 300;
inline constexpr size_t kUsbDisplayMono1Bytes =
    static_cast<size_t>(kUsbDisplayWidth) * kUsbDisplayHeight / 8U;

enum class UsbDisplayEncoding : uint8_t {
    Rgb565 = 0,
    Rgb888 = 1,
    Yuv420 = 2,
    Jpeg = 3,
    Mono1 = 4,
};

// Wire-compatible with the existing Windows xfz1986 USB Graphic driver.
// The legacy source labels this structure "20bytes", but its packed wire
// layout is 16 bytes; keep the executable ABI, not the stale comment.
struct __attribute__((packed)) UsbDisplayFrameHeader {
    uint16_t crc16;
    UsbDisplayEncoding encoding;
    uint8_t command;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint32_t frame_id_and_payload_total;
};

static_assert(sizeof(UsbDisplayFrameHeader) == 16,
              "USB Graphic frame header must remain 16 bytes");

constexpr uint32_t usb_display_frame_id(const UsbDisplayFrameHeader &header)
{
    return header.frame_id_and_payload_total & 0x3ffU;
}

constexpr uint32_t usb_display_payload_total(const UsbDisplayFrameHeader &header)
{
    return header.frame_id_and_payload_total >> 10U;
}

class UsbDisplayMono1Receiver {
public:
    bool begin(const UsbDisplayFrameHeader &header);
    bool append(const uint8_t *data, size_t length);
    bool active() const;
    bool complete() const;
    const uint8_t *data() const;
    size_t size() const;
    size_t remaining() const;
    void reset();

private:
    uint8_t frame_[kUsbDisplayMono1Bytes] = {};
    size_t received_ = 0;
    bool active_ = false;
};
