# RLCD Windows Control

This is the optional Windows control application for the Mono1 virtual display driver in [`../driver/`](../driver/). It configures the image-conversion mode and target frame-rate used by the installed driver; it does **not** flash firmware and does **not** switch Clock/Display mode on the device.

## Included

- `package/net8-x64/RLCD-Control-Net8-x64.exe`: compact x64 WinForms application.
- `RLCD-Control-v1.0.0.zip`: downloadable copy of the executable and this guide.
- `source/`: complete C# source.
- `Build-Control.ps1`: repeatable compact package build script.

## Use

1. Switch the board to **Display** with a long press of **KEY** (about 1.5 seconds).
2. Install the matching driver from [`../driver/README.md`](../driver/README.md).
3. Run `package/net8-x64/RLCD-Control-Net8-x64.exe`.
4. Select a mode and a target frame rate, then choose **应用到副屏**. Settings take effect on the next frame delivered by the virtual display driver.

The application writes `C:\ProgramData\RLCD-USB-Display\mode.ini`. If an older installation left that directory read-only for normal users, the app requests one UAC elevation only to grant the Users group modify permission, then retries the write.

Available modes: `Clear`, `Dark`, `Light`, `Photo`, `Invert`, `InvertPhoto`. Available target values: `10/15/20/24/30/40/50/60 FPS`. These are host conversion/pacing settings, not a promise that the RLCD panel itself will visibly reach every target frame rate.

Prerequisite: Windows 10/11 x64 with .NET 8 Windows Desktop Runtime. For a fully standalone executable, rebuild self-contained locally; the previous self-contained package exceeds GitHub's 100 MB per-file upload limit and is intentionally not tracked.

## Rebuild

```powershell
.\Build-Control.ps1
```
