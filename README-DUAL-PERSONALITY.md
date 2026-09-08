# 双 OTA 模式说明

本项目采用两个独立应用，而不是在同一固件内争抢 RLCD、Wi-Fi 和 USB 资源。

| 应用 | 分区 | 地址 | 用途 |
| --- | --- | --- | --- |
| Clock | `ota_0` | `0x20000` | 天气时钟、Wi-Fi、设置、音频、传感器与本地页面 |
| Display | `ota_1` | `0x6e0000` | Windows USB Mono1 副屏接收与面板呈现 |

完整分区表还包含 `nvs`、`otadata`、`phy_init`、`assets` 和 `model`。每个应用槽位为 `6912 KiB`；发布脚本将 `otadata` 初始化为 Clock，因此完整刷写后首次启动进入 Clock。

## 切换方式

1. **Clock → Display**：退出设置页后，长按 **KEY（GPIO18）约 1.5 秒**。Clock 设置 Display 分区为下次启动项并重启。
2. **Display → Clock**：长按 **KEY（GPIO18）约 1.5 秒**。Display 设置 `ota_0` 为下次启动项并重启。

切换过程中 USB 会断开并重新枚举。Windows 虚拟显示适配器仅在 Display 模式出现；切回 Clock 后适配器消失，这是预期结果。

## 镜像与完整刷写

`release/` 中六个文件组成一套匹配镜像：

```text
bootloader.bin
partition-table.bin
ota_data_initial.bin
RLCD-Clock-OTA0.bin
RLCD-USB-Display-OTA1.bin
srmodels.bin
```

进入下载模式后，从仓库根目录执行：

```powershell
Set-ExecutionPolicy -Scope Process Bypass
& .\tools\Flash-DualMode.ps1 -Port COM6
```

脚本使用 ESP-IDF v6.0.2 的 Python/esptool，目标为 ESP32-S3、16 MB Flash、DIO、80 MHz。它不覆盖 `0x9000` 的 NVS 分区，因此日常完整刷新会保留设备中的 Wi-Fi 与个人设置。刷写前后校验请见 [docs/LOCAL_TEST_AND_RELEASE.md](docs/LOCAL_TEST_AND_RELEASE.md)。

## 为什么这样设计

- Display 应用只保留稳定的 USB 收帧与面板呈现路径。
- Clock 应用保持原有网络、页面、音频与低功耗调度。
- 两套应用不在同一运行期访问面板，避免副屏帧与时钟绘制相互覆盖。
- 模式切换成本是一轮重启，换来可预测的 USB 枚举与面板所有权。