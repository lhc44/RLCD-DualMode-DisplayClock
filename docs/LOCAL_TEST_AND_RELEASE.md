# 本机构建、刷写与验收指南

本文适用于 `RLCD-DualMode-DisplayClock` 的统一固件：设备既可作为本地时钟，也可作为 Windows USB 副屏。

## 1. 发布前状态

本仓库当前已完成以下源码级工作：

- 固定 400×300 横向 RLCD、Mono1 全帧协议（每帧 `15,000` 字节）。
- TinyUSB Vendor 接收、帧长度/编码检查和原生面板呈现。
- `CLOCK` / `DISPLAY` 面板所有权互斥。
- 本地启动画面出现后，按住 **BOOT + KEY 1.5 秒** 在时钟与副屏之间切换；输入服务在联网/动画等待期间已可用。
- 返回时钟时自动完整重绘；副屏模式期间 LVGL 不写面板。
- 面板 SPI 为 `10 MHz`，与现有稳定副屏版本一致。

已完成目标 Xtensa 工具链的 Mono1 协议编译与链接检查。发布二进制前还应按第 5 节完成一次实机验收。

## 2. 环境要求

| 项目 | 要求 |
| --- | --- |
| 操作系统 | Windows 10/11 x64 |
| SDK | **ESP-IDF v6.0.2** |
| 目标 | `esp32s3` |
| 板卡 | Waveshare ESP32-S3-RLCD-4.2，16 MB Flash / 8 MB PSRAM |
| USB 驱动 | 已安装 RLCD Mono1 Windows 虚拟显示驱动 |

请在 ESP-IDF v6.0.2 的 PowerShell/命令提示符中执行以下操作。不要复用其他 SDK 生成的 `build` 目录。

## 3. 首次构建

```powershell
$repo = 'E:\weixue\RLCD-DualMode-DisplayClock\firmware-clock'
Set-Location $repo

# 仅清除本工程生成物；不影响源码和设备配置。
idf.py fullclean
idf.py set-target esp32s3
& .\tools\Build-IDF6.ps1
```

构建成功后，主要产物位于：

- `E:\weixue\RLCD-DualMode-DisplayClock\firmware-clock\build\weather_clock.bin`
- `E:\weixue\RLCD-DualMode-DisplayClock\firmware-clock\build\bootloader\bootloader.bin`
- `E:\weixue\RLCD-DualMode-DisplayClock\firmware-clock\build\partition_table\partition-table.bin`

工程默认配置为 16 MB Flash、QIO、80 MHz 和 Octal PSRAM。`Build-IDF6.ps1` 会在依赖解析后自动处理 ESP-DSP 1.7 与 IDF 6/Picolibc 的 `<cmath>` 兼容补丁。刷写时直接使用 `idf.py flash`，使其读取同一次构建生成的正确 `flash_args`。

## 4. 进入下载模式、刷写和串口日志

1. 断开板卡 USB。
2. 按住 **BOOT**。
3. 在保持 BOOT 的同时按一下 **PWR** 上电/复位。
4. 松开 BOOT，重新插入 USB 或等待串口出现。
5. 确认实际端口，例如 `COM6`。

```powershell
Set-Location E:\weixue\RLCD-DualMode-DisplayClock\firmware-clock
idf.py -p COM6 flash
idf.py -p COM6 monitor
```

退出串口监视器使用 `Ctrl+]`。首次刷写完成后，正常启动不需要按住 BOOT。

## 5. 实机验收清单

按顺序完成并记录结果；全部通过后再创建 GitHub Release。

### A. 时钟模式

- [ ] 断电重启后显示本地时钟，画面方向为横向 400×300。
- [ ] BOOT 短按切换下一个启用页面。
- [ ] KEY 短按进入设置，设置中 KEY 短按移动、BOOT 短按确认。
- [ ] 设置页中 KEY 长按返回一级/退出。
- [ ] 按住 BOOT 后按 PWR，设备进入 ROM 下载模式；正常运行时不存在下载模式误触发。

### B. 副屏模式

1. 先安装已验证的 Windows 驱动包：

   ```powershell
   Start-Process 'E:\weixue\rlcd-usb-display\final\RLCD-Driver-Setup.exe' -Verb RunAs
   ```

2. 完成安装后重新插拔板卡。在“设置 → 系统 → 显示”中确认出现 RLCD 虚拟显示器，选择“扩展这些显示器”。
3. 正常启动设备后按住 **BOOT + KEY 1.5 秒**，等待串口出现 `runtime display mode switched: 1`。
4. 将 Windows 窗口拖到 RLCD 虚拟显示器。检查文字、黑白边缘、方向和画面更新。
5. 再按住 **BOOT + KEY 1.5 秒**，串口应出现 `runtime display mode switched: 0`，并立刻完整回到本地时钟。

### C. 回归场景

- [ ] 在 `DISPLAY` 时拔插 USB，之后切回 `CLOCK`，本地时钟仍可用。
- [ ] 在 `CLOCK` 时拔插 USB，按键和时钟仍可用。
- [ ] 在 `DISPLAY` 下连续移动窗口/播放滚动内容，检查是否存在本地时钟与副屏交替抢写。
- [ ] 连续切换 10 次 `CLOCK ↔ DISPLAY`，每次回到时钟均无旧副屏残影。

## 6. Windows 副屏使用

现有稳定 Windows 包位于：

- `E:\weixue\rlcd-usb-display\final\RLCD-Driver-Setup.exe`
- `E:\weixue\rlcd-usb-display\final\RLCD-Control.exe`

驱动安装器会建立本机测试签名信任并安装 `USB\\VID_303A&PID_2986` 的 RLCD Mono1 虚拟显示驱动。控制程序可继续选择 Clear、Dark、Light、Photo、Invert、InvertPhoto 等主机图像转换模式。

统一固件的模式切换由板上按键决定：Windows 已创建副屏并不自动接管物理 RLCD；只有进入 `DISPLAY` 后，USB 全帧才会写入面板。

## 7. GitHub 发布前检查

```powershell
$repo = 'E:\weixue\RLCD-DualMode-DisplayClock'

git -C $repo status
git -C $repo fsck --full
git -C $repo log --oneline -10
```

应确保：

- `git status` 不显示待提交文件。
- 不提交 `firmware-clock/build/`、`managed_components/`、`release/`、串口日志或固件二进制。
- 不提交 Wi-Fi 密码、天气 API Key、API Host、Token、NVS 镜像、测试证书和 Windows 驱动安装输出。
- 保留 `LICENSE`、`NOTICE.md`、`UPSTREAM.md`、`firmware-clock/THIRD_PARTY_NOTICES.md`。

随后按 [`GITHUB_PUBLISH.md`](GITHUB_PUBLISH.md) 连接远程仓库并推送。
