# Dual-mode architecture

## Implemented runtime modes

- `CLOCK` (default): the imported local-clock firmware owns the ST7305/RLCD panel and preserves the original BOOT and KEY behavior.
- `DISPLAY`: the USB vendor transport accepts complete 400×300 Mono1 frames from the Windows virtual-display driver and is the sole panel presenter.
- `ECO`: reserved in the controller API for a later power-policy implementation; it is intentionally not selectable in this revision.

## State transition invariants

1. The ROM download gesture is physical **BOOT held while PWR powers/resets the board**. It occurs before application code and is never intercepted.
2. During normal operation, press and hold **BOOT + KEY for 1.5 seconds** to toggle `CLOCK` and `DISPLAY`.
3. In `DISPLAY`, LVGL still updates its retained scene but its flush callback completes without writing the panel. Only a validated full 15,000-byte Mono1 USB frame is presented.
4. Returning to `CLOCK` invalidates the active LVGL screen and performs a full redraw, so no stale USB image is retained.
5. A chord that completed a mode toggle is consumed: it neither changes a work page nor enters the settings page.
6. Normal single-key semantics remain: BOOT short press switches local pages / confirms settings; KEY short press enters or advances settings; KEY long press returns from settings.

## USB frame contract

| Field | Value |
| --- | --- |
| Panel coordinate space | 400×300 landscape |
| Accepted encoding | Mono1 (`15,000` bytes) |
| Update unit | complete full frame |
| Panel SPI clock | 10 MHz (same verified rate as the stable USB presenter) |
| VID/PID | `303A:2986` |
| USB interface | TinyUSB vendor bulk interface |

## Ownership

Only one presenter writes the physical ST7305 panel at a time. The local LVGL flush path is gated in `DISPLAY`; the USB receiver is gated outside `DISPLAY`.
