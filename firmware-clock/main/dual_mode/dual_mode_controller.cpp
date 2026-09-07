#include "dual_mode_controller.h"

#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

namespace {
constexpr uint32_t kRetainedModeMagic = 0x444D4F44U; // "DMOD"

// Preserve the selected panel role across watchdog/software resets.  It is
// intentionally RTC-backed rather than flash-backed: removing power returns
// to the documented default Clock mode, while a transient reset cannot undo a
// user-requested Display switch.
RTC_DATA_ATTR uint32_t s_retained_mode_magic = 0;
RTC_DATA_ATTR uint8_t s_retained_mode = static_cast<uint8_t>(DualMode::Clock);

portMUX_TYPE s_mode_lock = portMUX_INITIALIZER_UNLOCKED;
DualModeSnapshot s_snapshot = {
    .mode = DualMode::Clock,
    .reason = DualModeReason::Boot,
    .generation = 0,
};
}

void dual_mode_init()
{
    const bool retained_display =
        s_retained_mode_magic == kRetainedModeMagic &&
        s_retained_mode == static_cast<uint8_t>(DualMode::Display);
    portENTER_CRITICAL(&s_mode_lock);
    s_snapshot = {
        .mode = retained_display ? DualMode::Display : DualMode::Clock,
        .reason = DualModeReason::Boot,
        .generation = 1,
    };
    portEXIT_CRITICAL(&s_mode_lock);
}

DualModeSnapshot dual_mode_snapshot_load()
{
    portENTER_CRITICAL(&s_mode_lock);
    const DualModeSnapshot snapshot = s_snapshot;
    portEXIT_CRITICAL(&s_mode_lock);
    return snapshot;
}

bool dual_mode_request(DualMode target, DualModeReason reason)
{
    portENTER_CRITICAL(&s_mode_lock);
    if (s_snapshot.mode == target) {
        portEXIT_CRITICAL(&s_mode_lock);
        return false;
    }
    s_snapshot.mode = target;
    s_snapshot.reason = reason;
    ++s_snapshot.generation;
    s_retained_mode_magic = kRetainedModeMagic;
    s_retained_mode = static_cast<uint8_t>(target);
    portEXIT_CRITICAL(&s_mode_lock);
    return true;
}

bool dual_mode_toggle_from_runtime_chord()
{
    const DualMode mode = dual_mode_snapshot_load().mode;
    const DualMode target = mode == DualMode::Display ? DualMode::Clock : DualMode::Display;
    return dual_mode_request(target, DualModeReason::ButtonChord);
}
