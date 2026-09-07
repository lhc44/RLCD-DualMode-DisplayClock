#pragma once

class DisplayPort;

bool usb_display_service_init(DisplayPort &display);
bool usb_display_service_host_attached();

// Changes ownership of the physical panel. The Windows virtual adapter stays
// enumerated, so this operation does not reset the board or USB connection.
void usb_display_service_set_display_active(bool active);

// Requests that the USB task present the most recently received Windows frame.
// Windows intentionally suppresses identical frames, so this makes a physical
// Clock -> Display transition visible even while the desktop is static.
void usb_display_service_request_cached_frame();
