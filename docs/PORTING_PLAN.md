# Port and verification plan

## Completed integration work

1. Imported the complete clock baseline and preserved its upstream notices.
2. Added the local TinyUSB vendor component and the stable 400×300 Mono1 descriptor identity.
3. Ported the fixed-size USB Mono1 receiver and native `RLCD_PresentMono1()` presenter entry point.
4. Added a locked dual-mode controller, LVGL flush exclusion, and redraw-on-return behavior.
5. Added the post-startup BOOT+KEY 1.5-second chord with a consumed release path.

## Required target-hardware validation

1. Build the project with the upstream-compatible ESP-IDF 5.5.3 environment.
2. Flash and confirm default `CLOCK` boot, BOOT short-page switching, KEY settings navigation, and BOOT-held PWR download entry.
3. Hold BOOT+KEY for 1.5 seconds: Windows should become the active panel presenter; repeat to restore an immediately redrawn clock.
4. Send a known alternating Mono1 test frame and a full Windows desktop frame; verify the 400×300 landscape orientation and no concurrent clock writes.
5. Disconnect/reconnect USB while in `DISPLAY`, then toggle to `CLOCK` and confirm the local UI remains usable.

## Deferred work

- Eco-mode power transition and wake policy.
- Windows control-plane messages beyond the existing full-frame Mono1 data stream.
- Device-side frame-rate telemetry and host adaptive pacing.
