#pragma once

class DisplayPort;

bool usb_display_service_init(DisplayPort &display);
bool usb_display_service_host_attached();

// Requests that the USB task present the most recently received Windows frame.
// Windows intentionally suppresses identical frames, so this makes a physical
// Clock -> Display transition visible even while the desktop is static.
void usb_display_service_request_cached_frame();
