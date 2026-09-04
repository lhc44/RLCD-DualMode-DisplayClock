// Owns the high-level RLCD presentation state. Hardware presenters are
// registered in later port stages; this module deliberately has no display,
// USB, Wi-Fi, or LVGL dependency.
#pragma once

#include <stdint.h>

enum class DualMode : uint8_t {
    Clock = 0,
    Display = 1,
    Eco = 2,
};

enum class DualModeReason : uint8_t {
    Boot = 0,
    ButtonChord = 1,
    UsbHostAttached = 2,
    UsbHostDetached = 3,
    HostControl = 4,
};

struct DualModeSnapshot {
    DualMode mode;
    DualModeReason reason;
    uint32_t generation;
};

// Called once during normal app startup. It never observes BOOT/PWR startup
// signals, preserving the ROM download gesture.
void dual_mode_init();
DualModeSnapshot dual_mode_snapshot_load();
bool dual_mode_request(DualMode target, DualModeReason reason);
bool dual_mode_toggle_from_runtime_chord();

