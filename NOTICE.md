# Third-party and upstream notice

This repository incorporates the `firmware-clock/` source baseline from:

- `wickenzh/ESP32-S3-RLCD-4.2`
- Imported commit recorded in `docs/UPSTREAM.md`

The upstream non-commercial license is retained verbatim in the repository root
and in `firmware-clock/LICENSE`. Copyright notices and third-party notices in
`firmware-clock/THIRD_PARTY_NOTICES.md` remain part of this distribution.

`firmware-display/` is the dedicated `ota_1` USB Display application used by
this dual-OTA integration. `windows/driver/` contains the companion x64
IDDCX/UMDF driver source, reproducible local test-signing installer source and
driver payload; see `windows/driver/THIRD_PARTY_NOTICES.md` for its notices.
`windows/control/` contains the optional Windows mode/frame-rate control
application and its source. `reference/` contains local protocol and presenter
reference material only; it is not linked into either release application.
