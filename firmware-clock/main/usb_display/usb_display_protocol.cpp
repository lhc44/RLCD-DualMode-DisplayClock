#include "usb_display_protocol.h"

#include <string.h>

bool UsbDisplayMono1Receiver::begin(const UsbDisplayFrameHeader &header)
{
    reset();
    if (header.encoding != UsbDisplayEncoding::Mono1 ||
        header.width != kUsbDisplayWidth ||
        header.height != kUsbDisplayHeight ||
        usb_display_payload_total(header) != kUsbDisplayMono1Bytes) {
        return false;
    }
    active_ = true;
    return true;
}

bool UsbDisplayMono1Receiver::append(const uint8_t *data, size_t length)
{
    if (!active_ || !data || length > kUsbDisplayMono1Bytes - received_) {
        reset();
        return false;
    }
    memcpy(frame_ + received_, data, length);
    received_ += length;
    return true;
}

bool UsbDisplayMono1Receiver::active() const
{
    return active_;
}

bool UsbDisplayMono1Receiver::complete() const
{
    return active_ && received_ == kUsbDisplayMono1Bytes;
}

const uint8_t *UsbDisplayMono1Receiver::data() const
{
    return complete() ? frame_ : nullptr;
}

size_t UsbDisplayMono1Receiver::size() const
{
    return complete() ? received_ : 0;
}

void UsbDisplayMono1Receiver::reset()
{
    received_ = 0;
    active_ = false;
}
