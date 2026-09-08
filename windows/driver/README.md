# RLCD Mono1 Windows virtual display driver

This directory is the companion Windows x64 IDDCX driver for the Display firmware in this repository. It is **not a generic USB display driver**.

| Item | Value |
|---|---|
| Hardware ID | `USB\VID_303A&PID_2986` |
| Driver | `xfz1986_usb_graphic` 1.2.25.7 |
| Presentation contract | Mono1, 400 × 300 landscape, 15,000 bytes per full frame |
| Windows architecture | x64 only |
| Driver model | UMDF/IDDCX virtual display |

## Included material

- `source/`: curated Visual Studio/WDK driver source; open `idd_xfz1986_usb_graphic.sln`.
- `package/`: exact x64 driver payload (`.dll` + `.inf`) and checksum manifest.
- `setup-source/`: source for the elevated local test-signing installer.
- `package/net8-x64/RLCD-Driver-Setup-Net8-x64.exe`: compact setup executable built from `setup-source/`.
- `RLCD-Mono1-Driver-v1.2.25.7.zip`: downloadable package copy of the driver payload, installer and this guide.
- [`../control/`](../control/): optional Windows control program for selecting the conversion mode and target frame rate after driver installation.

No personal certificate, private signing key or pre-signed catalog is included. The setup program creates a **local test certificate** on the current PC, trusts it locally, uses the x64 WDK tools to generate/sign the catalog and invokes `pnputil` to install the driver. This is appropriate for local testing; use an authenticated production certificate for public release distribution.

## Install

Prerequisites: Windows 10/11 x64, .NET 8 Windows Desktop Runtime (for the compact EXE), and Windows SDK/WDK with x64 `Inf2Cat.exe` and `signtool.exe`.

1. Switch the device to **Display**: from Clock normal page, long-press **KEY** for about 1.5 seconds. It restarts into `ota_1`.
2. Run `package/net8-x64/RLCD-Driver-Setup-Net8-x64.exe` as Administrator.
3. Click install, unplug/replug the device if requested, then open Windows **Display settings**. Extend the new 400 × 300 display.
4. Verify the driver identity:
   ```powershell
   Get-PnpDevice -PresentOnly | Where-Object InstanceId -like 'USB\VID_303A&PID_2986*'
   ```

The installer deliberately selects the **x64** WDK binaries. This avoids the ARM64 `apivalidator.exe` architecture mismatch seen when a generic recursive tool search picks the wrong host-tool directory.

## Rebuild

- Driver: install Visual Studio 2022 C++ desktop tools + Windows 10/11 WDK; build `source/idd_xfz1986_usb_graphic.sln` as `Release|x64`.
- Compact setup EXE:
  ```powershell
  .\Build-Driver-Setup.ps1
  ```

## Boundaries

- The virtual display adapter appears only while the device is running the Display firmware. It disappears when the device returns to Clock; this is expected.
- The `.inf` matches only the hardware ID above. Do not bind it to unrelated USB devices.
- This package does not include the optional RLCD Control application; it installs the virtual display driver only.
