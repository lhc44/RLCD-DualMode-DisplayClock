#pragma once

class DisplayPort;

bool usb_display_service_init(DisplayPort &display);
bool usb_display_service_host_attached();

// Changes the USB PnP identity and reconnects the native USB port.  Windows
// creates/removes the indirect display adapter on this device-arrival event.
void usb_display_service_set_display_active(bool active);

// Requests that the USB task present the most recently received Windows frame.
// Windows intentionally suppresses identical frames, so this makes a physical
// Clock -> Display transition visible even while the desktop is static.
void usb_display_service_request_cached_frame();
