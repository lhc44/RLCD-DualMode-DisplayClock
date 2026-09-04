# Dual-mode architecture

## Runtime states

- `CLOCK`: full local clock firmware and documented BOOT/KEY interaction.
- `DISPLAY`: USB vendor display transport owns the RLCD framebuffer and presents Windows Mono1 frames.
- `ECO`: USB host detached; the clock scheduler may enter low-power waits and use minute-aligned partial updates.

## State transition invariants

1. Bootloader download gesture (`BOOT` held while `PWR` powers/resets the board) is never handled by application code.
2. `BOOT + KEY` is sampled only after normal startup and both GPIO inputs are stable.
3. Entering `DISPLAY` pauses local UI drawing, Wi-Fi work, audio work, and local RLCD flushes before host frames are accepted.
4. Returning to `CLOCK` drains host-frame work, invalidates the display cache, and redraws the selected local page.
5. Returning to `DISPLAY` sends a device-ready control status and requires a host full-frame redraw before differential delivery resumes.

## Ownership

Only one presenter owns the physical ST7305 panel at a time. The USB transport never writes a panel buffer while the local UI presenter holds it.
