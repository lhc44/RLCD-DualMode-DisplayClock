# Port sequence

1. Lock the clock base dependencies to ESP-IDF 5.5.3 and compile the unmodified baseline.
2. Add a dormant USB vendor transport component and validate that its descriptor does not change the clock's boot path.
3. Port the verified Mono1 frame parser and frame-owner/DMA serialization mechanism.
4. Add `CLOCK`, `DISPLAY`, and `ECO` state ownership with panel presenter exclusion.
5. Add post-startup BOOT+KEY chord detection with individual BOOT/KEY semantics preserved.
6. Extend the Windows driver protocol with `DEVICE_MODE`, `DISPLAY_READY`, and `FULL_REDRAW` control messages.
7. Perform display, key, power, USB disconnect, and download-mode regression runs.
