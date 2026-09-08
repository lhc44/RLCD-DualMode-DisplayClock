# 发布到 GitHub

## 发布边界

本仓库有意跟踪当前已验证的 `release/` 六文件镜像，以及 `windows/driver/` 中的 Windows x64 驱动和 `windows/control/` 中的模式/目标帧率控制程序，方便新用户完整刷写、安装与配置。构建缓存、私密设备配置和本机驱动签名材料不进入仓库。

| 应提交 | 不提交 |
| --- | --- |
| 源码、根 README、`docs/`、许可证和来源记录 | `firmware-*/build/`、`managed_components/`、`.build-cache/` |
| 当前匹配的 `release/` 六个镜像与 `SHA256SUMS.txt` | Wi-Fi 密码、天气 API Key、Token、NVS dump、串口日志 |
| 发布版本的校验值、`windows/driver/` 的源码/载荷/安装器、`windows/control/` 的源码/控制 EXE | 本机测试签名证书、私有签名密钥、构建缓存、私人截图 |

`.gitignore` 默认忽略新生成的 `release/` 文件。更新已跟踪的发布镜像后，复核 SHA-256；新增镜像时使用 `git add -f release/<file>`。

## 本地发布门禁

```powershell
$repo = '<repo-root>'
git -C $repo status
git -C $repo diff --check
git -C $repo fsck --full
Get-Content "$repo\release\SHA256SUMS.txt"
Get-FileHash -Algorithm SHA256 "$repo\release\*" | Format-Table Path, Hash
```

继续前满足以下条件：

- 工作区已提交；
- `release/SHA256SUMS.txt` 与六个文件一致；
- `windows/driver/package/SHA256SUMS.txt` 与 `.dll`、`.inf` 一致，紧凑安装器与 ZIP 可打开；
- `windows/control/package/SHA256SUMS.txt` 与控制 EXE 一致，控制 ZIP 可打开；

- 已完成 [LOCAL_TEST_AND_RELEASE.md](LOCAL_TEST_AND_RELEASE.md) 的 Clock、Display 与切换验收；
- README 中的按键和配网描述与实际固件一致。

## 新建远程并推送

GitHub 账户邮箱不等于 GitHub 用户名。请从 GitHub 个人主页或新建仓库页面复制实际仓库 HTTPS URL。

```powershell
$repo = '<repo-root>'
$remote = 'https://github.com/<github-username>/RLCD-DualMode-DisplayClock.git'

git -C $repo remote add origin $remote
git -C $repo branch -M main
git -C $repo push -u origin main
```

若 `origin` 已存在，改用：

```powershell
git -C $repo remote set-url origin $remote
git -C $repo push -u origin main
```

GitHub 的密码输入框使用 Personal Access Token（PAT），而不是账户登录密码。Fine-grained token 选择该仓库，并授予 `Contents: Read and write`。

## 创建 Release

从经过硬件验收的 `main` 提交打标签：

```powershell
$repo = '<repo-root>'
git -C $repo tag -a v0.1.0 -m 'Initial verified dual OTA release'
git -C $repo push origin main --tags
```

Release Notes 至少列出：硬件型号、ESP-IDF 版本、六个镜像的 SHA-256、Clock/Display 的 KEY 长按切换方式、`windows/driver/` 的 x64 测试签名安装前提、`windows/control/` 的模式/目标帧率配置作用、2.4 GHz Wi-Fi 限制、已知 RLCD 画面边界，以及完整刷写回退步骤。
