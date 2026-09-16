# Upstream baselines

## Clock baseline

`firmware-clock/` is imported from [`wickenzh/ESP32-S3-RLCD-4.2`](https://github.com/wickenzh/ESP32-S3-RLCD-4.2) at commit `6a5d19ba0880a4e0772a5c763e85c75c4954468b` (2026-09-13), upstream `v1.6.5`. It supplies the local clock, UI, RTC, sensors, power management, audio and networking baseline. The local dual-image integration disables its single-image in-device OTA path so it cannot overwrite `ota_1` (Display).

## USB display baseline

`firmware-display/` is the dedicated USB Display application used for `ota_1`. It implements the ESP32-S3 USB Vendor transport, the 400×300 Mono1 presentation path and the return-to-Clock KEY long-press action. It is built and flashed independently from `firmware-clock/`.

The local `reference/` material records prior protocol/presenter work and is not part of either release CMake application. The companion Windows driver source and install payload are maintained in `windows/driver/`; see its README and third-party notice.

See [docs/UPSTREAM.md](docs/UPSTREAM.md) for the detailed import record.
