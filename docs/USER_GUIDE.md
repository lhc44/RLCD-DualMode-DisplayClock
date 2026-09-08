# 使用指南：Clock、配网与 Windows 副屏

## 上电后你会看到什么

完整刷写后，设备默认进入 **Clock**。Clock 是独立的 `ota_0` 应用；它会显示启动动画，然后进入时钟页面并按已保存配置连接网络。

长按 KEY 切到 **Display** 后，设备会重启，Windows 才会枚举到虚拟显示器。Display 是独立的 `ota_1` 应用，屏幕接收 Windows 送来的黑白帧，不显示 Clock 页面。

## 按键

| 操作 | 结果 |
| --- | --- |
| Clock：BOOT 短按 | 下一个已启用页面 |
| Clock：KEY 短按 | 打开设置；在设置内移动选择 |
| Clock 设置：BOOT 短按 | 确认当前项目 |
| Clock 设置：KEY 长按 | 返回一级或退出设置 |
| Clock 普通页面：KEY 长按约 1.5 秒 | 重启到 Display |
| Display：KEY 长按约 1.5 秒 | 重启回 Clock |
| 按住 BOOT，再短按 PWR | 进入下载模式 |

切换模式前先退出设置页。Clock 的 KEY 长按在设置页保留“返回”语义，不触发模式切换。

## 配置或更换 Wi-Fi

ESP32-S3 只连接 **2.4 GHz Wi-Fi**。

### 首次配网或更换网络

1. 保持设备在 Clock。
2. KEY 短按打开设置，选择 **网络**。
3. 选择第一项 **更换 Wi-Fi**，再按 BOOT 确认。
4. 屏幕提示后，手机或电脑连接临时热点：
   - SSID：`WeatherClock-XXXX`，`XXXX` 为设备 MAC 地址的末两字节；
   - 密码：`12345678`。
5. 浏览器打开 `http://192.168.4.1/`，选择或填写主 Wi-Fi；备用 Wi-Fi 为选填。
6. 保存，等待设备验证并切换到新网络。

“更换 Wi-Fi”只更新主/备 Wi-Fi 凭据。天气 API Key、API Host、城市、页面、闹钟、相册、音量与其他本地设置不会被清除。配网页仍会显示天气配置字段；已有配置会沿用。

### 临时热点没有显示

- 确认当前处于 Clock，而不是 Display。
- 再次进入 `设置 → 网络 → 更换 Wi-Fi`。
- 在 Clock 的系统设置中，恢复出厂设置会清除全部设备配置；它用于彻底重置，不是日常换路由器的首选路径。

当前固件的网页仅在配网热点运行期间存在；设备连上家庭 Wi-Fi 后，没有常驻局域网管理网页。

## Clock 功能

默认工作页包括：天气时钟、图片时钟、天气看板、温湿时钟、日历、温湿历史和小智 AI。可在 `设置 → 显示` 中开关和排序页面，系统始终保留至少一个页面。

- `设置 → 网络`：更换 Wi-Fi、同步时间、同步天气、更新一言、天气城市。
- `设置 → 声音`：音量、声音选择、整点提醒、全天提醒。
- `设置 → 显示`：页面开关、页面顺序、小智节能、闹钟、图片切换。
- `设置 → 系统`：离线模式、网络检测、恢复出厂、设备信息、检查更新。

音量循环顺序为 `0 → 5 → 10 → 20 → 40 → 60 → 80 → 100 → 0`。`0%` 为全局静音：整点提醒、闹钟、番茄钟和小智扬声器不会出声；Wi-Fi 配网提示音是独立路径。

## Windows 副屏

1. 从仓库 [`windows/driver/`](../windows/driver/) 以管理员身份运行 `package/net8-x64/RLCD-Driver-Setup-Net8-x64.exe`，安装匹配本固件的 RLCD Mono1 虚拟显示驱动。该紧凑安装器需要 .NET 8 Windows Desktop Runtime 与 Windows SDK/WDK 的 x64 签名工具。
2. 用 USB 数据线连接设备；先退出 Clock 设置页。
3. Clock 普通页面中长按 KEY 约 1.5 秒，设备重启进入 Display。
4. Windows“显示设置”中应出现一个 `400 × 300` 横向虚拟显示器；选择“扩展这些显示器”，并将缩放设置为 `100%`。
5. 运行 [`windows/control/`](../windows/control/) 中的 `package/net8-x64/RLCD-Control-Net8-x64.exe`，选择 `Clear`、`Dark`、`Light`、`Photo`、`Invert` 或 `InvertPhoto`，并选择目标帧率后点击“应用到副屏”。该程序只配置 Windows 驱动，下一帧刷新时生效。
6. 将窗口拖到该显示器上。
7. 需要回到 Clock 时，在 Display 长按 KEY 约 1.5 秒。

Display 走 Mono1 黑白数据，适合文字、状态页、静态工具和低频动画。为减少残影与闪烁，Windows 主机侧图像转换应优先使用适合 RLCD 的清晰黑白模式；高速视频和连续灰阶抖动不属于该屏幕的最佳使用方式。

### Windows 找不到副屏

1. 确认设备已经重启到 Display；Clock 下不会创建副屏适配器。
2. 确认 USB 数据线具备数据功能。
3. 确认 [`windows/driver/`](../windows/driver/) 中的虚拟显示驱动已安装，且和 `VID_303A:PID_2986` 匹配。
4. 在设备管理器重新插拔设备，随后重新打开“显示设置”。

## 下载模式与固件恢复

下载模式手势是：**按住 BOOT → 短按 PWR → 松开 BOOT**。随后运行仓库根目录的完整刷写脚本：

```powershell
Set-ExecutionPolicy -Scope Process Bypass
& .\tools\Flash-DualMode.ps1 -Port COM6
```

脚本刷完后重新从 Clock 启动。请先核对 [release/SHA256SUMS.txt](../release/SHA256SUMS.txt) 的文件校验值。