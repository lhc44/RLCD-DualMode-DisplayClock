#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// The physical USB device must re-enumerate when its Windows role changes.
// PID 0x2986 is the IDD driver's hardware ID; the clock PID deliberately has
// no IDD binding.
bool usb_display_service_enumerate_as_display(void);

#ifdef __cplusplus
}
#endif
