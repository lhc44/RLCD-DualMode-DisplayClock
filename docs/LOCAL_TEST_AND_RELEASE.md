# 构建、刷写、验证与发布

本文以当前项目的双 OTA 发布布局为准。日常使用预编译 `release/` 镜像；维护者才需要重建两个应用。

## 发布前置条件

| 项目 | 要求 |
| --- | --- |
| 操作系统 | Windows 10/11 x64 |
| SDK | ESP-IDF `v6.0.2` |
| 目标 | `esp32s3` |
| 板卡 | Waveshare ESP32-S3-RLCD-4.2（16 MB Flash / 8 MB PSRAM） |
| Python | ESP-IDF 6.0.2 安装附带的 venv |

所有命令从仓库根目录执行。`firmware-clock/tools/Build-IDF6.ps1` 会配置 IDF 6 环境并为受影响的 ESP-DSP 源文件补入兼容性头文件；不要混用其他 ESP-IDF 版本生成的 `build/` 目录。

## 1. 校验发布镜像

```powershell
Set-Location <repo-root>
Get-FileHash -Algorithm SHA256 .\release\* | Format-Table Path, Hash
Get-Content .\release\SHA256SUMS.txt
```

校验值必须与 `release/SHA256SUMS.txt` 一致。六个文件缺少任意一个时，不要执行完整刷写。

## 2. 完整刷写（推荐）

1. 拔下板子。
2. 按住 **BOOT**。
3. 保持 BOOT 的同时短按 **PWR**。
4. 松开 BOOT，确认下载端口，例如 `COM6`。
5. 执行：

   ```powershell
   Set-ExecutionPolicy -Scope Process Bypass
   & .\tools\Flash-DualMode.ps1 -Port COM6
   ```

该脚本写入 `0x0`、`0x8000`、`0xF000`、`0x20000`、`0x6E0000` 和 `0xFA0000`，不写 `0x9000` 的 NVS。刷写结束后设备会从 Clock (`ota_0`) 启动。

## 3. 构建 Clock（维护者）

```powershell
Set-Location <repo-root>\firmware-clock
Set-ExecutionPolicy -Scope Process Bypass
& .\tools\Build-IDF6.ps1
```

Clock 产物：

```text
firmware-clock\build\weather_clock.bin
firmware-clock\build\bootloader\bootloader.bin
firmware-clock\build\partition_table\partition-table.bin
```

将 Clock 产物作为发布镜像前，必须同时检查它仍小于 `ota_0` 的 `6912 KiB` 槽位，并完整验证 Clock 与 Display 的切换。

## 4. 构建 Display（维护者）

Display 是独立 ESP-IDF 项目，工程名为 `usb_touch_screen`：

```powershell
Set-Location <repo-root>\firmware-display
idf.py set-target esp32s3
idf.py build
```

它的应用产物应与当前发布布局一起写入 `ota_1` (`0x6E0000`)。发布前，使用相同 SDK、相同分区表配置和一整套新的 SHA-256 清单；不要把单独构建的任意一个应用与旧套件混合发布。

## 5. 硬件验收

### Clock

- [ ] 完整刷写后进入 Clock。
- [ ] BOOT 短按切页；KEY 短按可进入设置。
- [ ] `设置 → 网络 → 更换 Wi-Fi` 能启动配网热点。
- [ ] KEY 长按在设置页执行返回；退出设置页后长按 KEY 会重启到 Display。
- [ ] 音量可循环至 `0%`，重启后仍保持该值。

### Display

- [ ] 长按 KEY 后设备重启，Windows 枚举 `VID_303A:PID_2986` 对应的虚拟显示适配器。
- [ ] Windows 显示设置中该显示器为 `400×300` 横向。
- [ ] 文本、黑白边缘与静态画面清晰；连续内容没有时钟页面覆盖。
- [ ] Display 长按 KEY 后重新进入 Clock，Windows 副屏适配器消失。

### 回归与恢复

- [ ] 连续完成至少 10 次 Clock ↔ Display 切换。
- [ ] 在 Display 拔插 USB 后，长按 KEY 仍可返回 Clock。
- [ ] 再次完整刷写后确认现有 NVS 设置仍保留；如需清空设置，在 Clock 使用恢复出厂设置。

## 6. 回退

完整刷写会把两槽恢复为仓库 `release/` 中对应的一套镜像，并将启动项初始化为 Clock。若正在测试新构建，保留上一个已验证的六文件发布目录和 SHA-256 清单，即可按同一下载模式步骤恢复。