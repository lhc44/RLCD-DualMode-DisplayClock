# RLCD Dual Personality Firmware

This release uses two independent ESP32-S3 application slots:

- `ota_0` at `0x20000`: original clock/weather firmware, including Wi-Fi and the original user interface.
- `ota_1` at `0x6e0000`: the proven native USB Mono1 display firmware.

They never run together. This separates Wi-Fi/LVGL/weather from the high-rate USB display transport and avoids shared-RLCD SPI ownership conflicts.

## Flash both slots

Put the board into download mode, then run from PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
& "E:\weixue\RLCD-DualMode-DisplayClock\tools\Flash-DualMode.ps1" -Port COM6
```

After the flash it boots clock mode (`ota_0`). Long-press **KEY** (GPIO18) for about 1.5 seconds to reboot into USB display mode (`ota_1`). Long-press **KEY** again in USB display mode to reboot back into clock mode.

The USB display driver binds only while `ota_1` is running. It exposes the native 400x300 Mono1 secondary display as before.
