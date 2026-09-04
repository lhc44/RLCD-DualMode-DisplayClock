# RLCD DualMode DisplayClock

ESP32-S3-RLCD-4.2 unified firmware: Windows USB secondary display, local clock, and battery-oriented eco mode.

## Status

Foundation repository. The first milestone preserves the existing 400×300 Mono1 Windows secondary-display transport while importing the local-clock project as a separately tracked upstream baseline.

## Repository layout

- `firmware-clock/`: imported full clock firmware baseline and all future unified firmware changes.
- `reference/usb-display-stable/`: verified USB display source reference.
- `docs/ARCHITECTURE.md`: runtime-state and presenter ownership rules.
- `docs/PORTING_PLAN.md`: ordered port and validation plan.
- `docs/GITHUB_PUBLISH.md`: clean public-publishing checklist.

## License and attribution

This project retains the imported upstream license and notices in `LICENSE`,
`NOTICE.md`, and `firmware-clock/THIRD_PARTY_NOTICES.md`.
