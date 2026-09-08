# RLCD DualMode DisplayClock

> ESP32-S3-RLCD-4.2 的双固件桌面时钟与 Windows USB 副屏项目。中文文档为准。

本项目将 Waveshare ESP32-S3-RLCD-4.2 分成两个**独立 OTA 应用**：本地天气时钟与 Windows USB 副屏。两者不会在同一次启动中并发运行，因此时钟的 Wi-Fi/LVGL/音频栈和副屏的高频 USB 传输彼此隔离。

## 亮点

- **时钟模式**：天气、日历、图片、温湿度、历史数据、小智 AI、闹钟、番茄钟和整点提醒。
- **副屏模式**：Windows 虚拟显示驱动通过 USB Vendor 接口传送 `400×300` 横向 Mono1 全帧；面板实际帧大小为 `15,000 bytes`。
- **稳定切换**：长按板载 **KEY（GPIO18）约 1.5 秒**，设备写入下次启动分区并重启到另一模式。
- **配网保留设置**：`设置 → 网络 → 更换 Wi-Fi` 打开临时配网热点；只更新 Wi-Fi 凭据，页面、闹钟、相册、音量和天气配置保持原样。
- **音量档位**：`0 / 5 / 10 / 20 / 40 / 60 / 80 / 100%`；`0%` 是持久化的全局静音。

## 先读这里

| 文档 | 内容 |
| --- | --- |
| [docs/USER_GUIDE.md](docs/USER_GUIDE.md) | 日常使用、按键、配网、Windows 副屏与排障 |
| [README-DUAL-PERSONALITY.md](README-DUAL-PERSONALITY.md) | 两个 OTA 应用与刷写包说明 |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | 分区、模式切换、USB 协议和资源边界 |
| [docs/LOCAL_TEST_AND_RELEASE.md](docs/LOCAL_TEST_AND_RELEASE.md) | 构建、刷写、校验、回退与硬件验收 |
| [docs/GITHUB_PUBLISH.md](docs/GITHUB_PUBLISH.md) | 推送 GitHub 与发布检查表 |
| [docs/PORTING_PLAN.md](docs/PORTING_PLAN.md) | 已完成项目状态、限制与后续方向 |
| [release/SHA256SUMS.txt](release/SHA256SUMS.txt) | 当前发布镜像的校验值 |

## 支持的硬件与软件

| 项目 | 当前发布基线 |
| --- | --- |
| 开发板 | Waveshare ESP32-S3-RLCD-4.2 |
| MCU | ESP32-S3 |
| Flash / PSRAM | 16 MB Flash / 8 MB PSRAM |
| 面板输出 | `400×300` 横向，Mono1，`15,000 bytes/frame` |
| Wi-Fi | 2.4 GHz（ESP32-S3 硬件能力） |
| 构建工具链 | ESP-IDF `v6.0.2` |
| Windows 副屏 | 匹配 `VID_303A:PID_2986` 的外部 Windows 虚拟显示驱动 |

本仓库只发布设备端固件与源码；Windows 驱动/控制程序属于配套主机项目，安装包需与上表 USB 身份及 Mono1 协议匹配。副屏模式不是免驱 HID/USB 显示器，Windows 端需要安装对应虚拟显示驱动。

## 最快开始：使用已验证的发布镜像

1. 下载或克隆本仓库，保持 `release/` 目录完整。
2. 将板子置于下载模式：**按住 BOOT，再短按 PWR**；随后松开 BOOT。
3. 在 PowerShell 执行：

   ```powershell
   Set-ExecutionPolicy -Scope Process Bypass
   & .\tools\Flash-DualMode.ps1 -Port COM6
   ```

4. 首次刷写完成后会从 `ota_0` 启动到 Clock。
5. 在 Clock 模式长按 **KEY** 约 1.5 秒，设备重启进入 Display；再次长按 KEY 返回 Clock。

`Flash-DualMode.ps1` 写入 bootloader、分区表、双 OTA 应用、初始 OTA 数据和语音模型分区；脚本不写 NVS，因此已保存的 Wi-Fi、闹钟和其他本地设置会保留。详细步骤、校验和恢复方式见 [本地验收指南](docs/LOCAL_TEST_AND_RELEASE.md)。

## 日常操作速览

| 场景 | 操作 |
| --- | --- |
| Clock 页面轮换 | BOOT 短按 |
| 打开/移动设置 | KEY 短按 |
| 设置页返回 | KEY 长按 |
| 设置项确认 | BOOT 短按 |
| Clock → Display | 不在设置页时长按 KEY 约 1.5 秒，随后自动重启 |
| Display → Clock | 长按 KEY 约 1.5 秒，随后自动重启 |
| 进入下载模式 | 按住 BOOT 后短按 PWR |
| 更换家庭 Wi-Fi | Clock：`设置 → 网络 → 更换 Wi-Fi` |

模式切换会使 USB 设备重新枚举：只有 Display 模式下 Windows 才会看到虚拟显示适配器；Clock 模式下它不会出现。这是双应用隔离的正常行为。

## 仓库结构

```text
firmware-clock/       Clock（ota_0）源码与原始时钟能力
firmware-display/     USB Display（ota_1）源码
release/              已验证的完整刷写镜像与 SHA-256 清单
tools/                双模式完整刷写脚本
docs/                 面向用户、维护者和发布者的项目文档
upstream/clock-base/  导入时钟基线的本地记录（默认不提交）
```

## 当前边界

- 两个模式以重启切换，不提供同一启动中同时运行 Clock 与副屏的路径。
- 配网页是临时热点门户，不是常驻局域网控制台。
- `0%` 为全局静音，整点提醒、闹钟、番茄钟及小智扬声器都会静音；Wi-Fi 配网提示音走独立播放路径。
- RLCD 适合静态信息与低频更新；高速运动视频和持续灰阶抖动不属于目标场景。

## 许可证与来源

`firmware-clock/` 源自 [wickenzh/ESP32-S3-RLCD-4.2](https://github.com/wickenzh/ESP32-S3-RLCD-4.2)，导入提交为 [`9e9560a5f8133af429bf3631ee7e2e3cb7837f89`](docs/UPSTREAM.md)。保留的许可证、第三方声明和项目来源见 [LICENSE](LICENSE)、[NOTICE.md](NOTICE.md)、[UPSTREAM.md](UPSTREAM.md) 与 [firmware-clock/THIRD_PARTY_NOTICES.md](firmware-clock/THIRD_PARTY_NOTICES.md)。