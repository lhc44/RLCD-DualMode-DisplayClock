# RLCD DualMode DisplayClock

ESP32-S3-RLCD-4.2 unified firmware that keeps the original local clock and can hand the physical panel to a Windows USB secondary-display stream.

## Implemented integration

- Native panel: **400×300 landscape**, packed Mono1 (`15,000` bytes/frame).
- USB display ingress: TinyUSB vendor interface retaining the existing `303A:2986` device identity.
- Panel ownership: `CLOCK` at boot; hold **BOOT + KEY for 1.5 seconds** after boot to toggle `CLOCK` ↔ `DISPLAY`.
- Clock return: mode switching back to `CLOCK` invalidates and fully redraws the local scene.
- Download entry is unchanged: hold **BOOT**, then power/reset with **PWR**.

## Repository layout

- `firmware-clock/`: the unified firmware source.
- `reference/usb-display-stable/`: stable USB display source kept as a protocol reference.
- `docs/ARCHITECTURE.md`: exact ownership and input rules.
- `docs/PORTING_PLAN.md`: hardware validation and deferred items.
- `docs/GITHUB_PUBLISH.md`: clean public-publishing checklist.

## Build baseline

The imported clock project targets **ESP-IDF 5.5.3**. Use that matching SDK for release builds; the local ESP-IDF 6.0.2 installation is useful for source/toolchain checks but is not the release baseline.

## License and attribution

This project retains the imported upstream license and notices in `LICENSE`, `NOTICE.md`, and `firmware-clock/THIRD_PARTY_NOTICES.md`.
