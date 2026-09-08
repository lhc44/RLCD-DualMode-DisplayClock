# 架构与运行边界

## 两个应用，而非同一运行期双模式

设备 Flash 使用双 OTA 应用布局。`ota_0` 是 Clock，`ota_1` 是 Display；运行中的应用通过 `esp_ota_set_boot_partition()` 选择另一槽位，再调用 `esp_restart()`。因此同一时刻只有一个应用拥有 RLCD。

```text
Clock (ota_0) -- KEY long --> set ota_1 + reboot --> Display (ota_1)
Display (ota_1) -- KEY long --> set ota_0 + reboot --> Clock (ota_0)
```

这条路径与 ESP-ROM 下载模式完全独立：下载模式是硬件级的“按住 BOOT，再按 PWR”，发生在应用代码之前。

## 分区与发布映射

| Flash 地址 | 分区 | 发布文件 | 说明 |
| ---: | --- | --- | --- |
| `0x00000` | bootloader | `bootloader.bin` | ESP32-S3 引导程序 |
| `0x08000` | partition table | `partition-table.bin` | 固定的双 OTA 布局 |
| `0x0F000` | otadata | `ota_data_initial.bin` | 完整刷写后默认启动 `ota_0` |
| `0x20000` | `ota_0` | `RLCD-Clock-OTA0.bin` | Clock 应用 |
| `0x6E0000` | `ota_1` | `RLCD-USB-Display-OTA1.bin` | Display 应用 |
| `0xFA0000` | model | `srmodels.bin` | Clock 的语音模型数据 |

`nvs` 位于 `0x9000`，不在完整刷写脚本的写入列表中。此设计使更新固件时保留 Wi-Fi、天气配置、页面顺序、音量、闹钟和用户设置。

## Display 数据面

| 字段 | 值 |
| --- | --- |
| 物理面板坐标 | `400×300` 横向 |
| 帧编码 | packed Mono1 |
| 帧大小 | `15,000 bytes` |
| 传输接口 | TinyUSB Vendor bulk |
| USB 身份 | `VID_303A:PID_2986` |
| 面板写入者 | 仅 Display 应用 |

Display 不提供 USB 视频类（UVC）或标准 USB 显示类接口。Windows 需要匹配该 Vendor 协议的虚拟显示驱动；驱动枚举与副屏图像传输只在 Display 应用运行时发生。

## Clock 数据面

Clock 保留上游天气时钟的页面、传感器、RTC、音频、配网、天气和 OTA 能力。其设置菜单的“更换 Wi-Fi”仅请求临时 AP+STA 配网门户；保存新凭据后门户退出。项目当前没有常驻局域网 Web 控制台。

Clock 中的音量为共享全局音量，持久化档位为 `0/5/10/20/40/60/80/100%`。`0%` 对整点提醒、闹钟、番茄钟和小智扬声器生效；Wi-Fi 配网提示音使用独立播放调用。

## 输入契约

| 位置 | BOOT | KEY |
| --- | --- | --- |
| Clock 普通页面 | 短按切换下一个启用页面 | 短按进入设置；长按约 1.5 秒切到 Display |
| Clock 设置页 | 短按确认 | 短按移动；长按返回/退出设置 |
| Display | ROM 下载手势仍由硬件处理 | 长按约 1.5 秒切回 Clock |
| 任意模式 | 按住 BOOT 后按 PWR：进入下载模式 | — |

闹钟或番茄钟提示音播放时，按键会先停止本次提示音；该次按键不会再继续执行页面或设置动作。