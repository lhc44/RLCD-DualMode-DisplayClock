#include "dual_mode_controller.h"

#include "esp_attr.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "nvs.h"

namespace {
constexpr uint32_t kRetainedModeMagic = 0x444D4F44U; // "DMOD"
constexpr char kNvsNamespace[] = "dual_mode";
constexpr char kNvsModeKey[] = "mode";
constexpr char kTag[] = "dual_mode";

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

bool load_persistent_display_mode(bool fallback)
{
    nvs_handle_t handle = 0;
    if (nvs_open(kNvsNamespace, NVS_READONLY, &handle) != ESP_OK) {
        return fallback;
    }
    uint8_t stored = 0;
    const esp_err_t err = nvs_get_u8(handle, kNvsModeKey, &stored);
    nvs_close(handle);
    if (err != ESP_OK) {
        return fallback;
    }
    return stored == static_cast<uint8_t>(DualMode::Display);
}

void persist_mode(DualMode mode)
{
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(kNvsNamespace, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        err = nvs_set_u8(handle, kNvsModeKey, static_cast<uint8_t>(mode));
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    if (handle != 0) {
        nvs_close(handle);
    }
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "persist selected mode failed: %s", esp_err_to_name(err));
    }
}
}

void dual_mode_init()
{
    const bool retained_display =
        s_retained_mode_magic == kRetainedModeMagic &&
        s_retained_mode == static_cast<uint8_t>(DualMode::Display);
    // The physical KEY's long-press path can reset the board on some RLCD
    // revisions.  Keep the selected role in NVS as well as RTC memory, so
    // an immediate board reset after the gesture still reaches the selected
    // USB identity on the next boot.
    const bool selected_display = load_persistent_display_mode(retained_display);
    portENTER_CRITICAL(&s_mode_lock);
    s_snapshot = {
        .mode = selected_display ? DualMode::Display : DualMode::Clock,
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
    // Commit before requesting USB re-enumeration. This is deliberately
    // outside the critical section because NVS may touch flash.
    persist_mode(target);
    return true;
}

bool dual_mode_toggle_from_runtime_chord()
{
    const DualMode mode = dual_mode_snapshot_load().mode;
    const DualMode target = mode == DualMode::Display ? DualMode::Clock : DualMode::Display;
    return dual_mode_request(target, DualModeReason::ButtonChord);
}
