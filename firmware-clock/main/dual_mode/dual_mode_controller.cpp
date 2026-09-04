#include "dual_mode_controller.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

namespace {
portMUX_TYPE s_mode_lock = portMUX_INITIALIZER_UNLOCKED;
DualModeSnapshot s_snapshot = {
    .mode = DualMode::Clock,
    .reason = DualModeReason::Boot,
    .generation = 0,
};
}

void dual_mode_init()
{
    portENTER_CRITICAL(&s_mode_lock);
    s_snapshot = {
        .mode = DualMode::Clock,
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
    portEXIT_CRITICAL(&s_mode_lock);
    return true;
}

bool dual_mode_toggle_from_runtime_chord()
{
    const DualMode mode = dual_mode_snapshot_load().mode;
    const DualMode target = mode == DualMode::Display ? DualMode::Clock : DualMode::Display;
    return dual_mode_request(target, DualModeReason::ButtonChord);
}

