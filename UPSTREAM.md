# Upstream baselines

## Clock baseline

`firmware-clock/` is imported from `wickenzh/ESP32-S3-RLCD-4.2`, commit recorded in `UPSTREAM.md`.
It is the source baseline for local clock, UI, settings, RTC, sensors, power management, audio, networking, and OTA.

## USB display baseline

`reference/usb-display-stable/` is the verified USB Graphic implementation used by this project before the unified port. It contains the ESP-IDF source definitions for the 400×300 Mono1 vendor protocol, ST7305 presentation path, DMA completion serialization, and stable buffer ownership model.

It is a reference port, not a second application included by CMake. New unified code lands under `firmware-clock/` only.
