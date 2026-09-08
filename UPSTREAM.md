# Upstream baselines

## Clock baseline

`firmware-clock/` is imported from [`wickenzh/ESP32-S3-RLCD-4.2`](https://github.com/wickenzh/ESP32-S3-RLCD-4.2) at commit `9e9560a5f8133af429bf3631ee7e2e3cb7837f89` (2026-08-31). It supplies the local clock, UI, RTC, sensors, power management, audio, networking and OTA baseline.

## USB display baseline

`firmware-display/` is the dedicated USB Display application used for `ota_1`. It implements the ESP32-S3 USB Vendor transport, the 400×300 Mono1 presentation path and the return-to-Clock KEY long-press action. It is built and flashed independently from `firmware-clock/`.

The local `reference/` material records prior protocol/presenter work and is not part of either release CMake application.

See [docs/UPSTREAM.md](docs/UPSTREAM.md) for the detailed import record.