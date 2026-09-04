# Publishing to GitHub

## 1. Publishable repository boundary

The public source repository contains source code, notices, documentation, and repeatable build instructions. Keep local binaries, build caches, private device configuration, and test-signing material outside it.

Required tracked files:

- `LICENSE`
- `NOTICE.md`
- `UPSTREAM.md`
- `docs/ARCHITECTURE.md`
- `docs/LOCAL_TEST_AND_RELEASE.md`
- `firmware-clock/THIRD_PARTY_NOTICES.md`

Excluded material:

- `firmware-clock/build/`, `firmware-clock/managed_components/`, `release/`
- Wi-Fi passwords, weather API keys, private API hosts, tokens, NVS images
- serial captures, screenshots that include private content, test certificates
- generated Windows driver packages and local driver-signing files

## 2. Local pre-push gate

```powershell
$repo = 'E:\weixue\RLCD-DualMode-DisplayClock'

git -C $repo status
git -C $repo diff --check
git -C $repo fsck --full
git -C $repo log --oneline -10
```

The working tree must be clean. Then complete the hardware checklist in [`LOCAL_TEST_AND_RELEASE.md`](LOCAL_TEST_AND_RELEASE.md), including both `CLOCK` and `DISPLAY` mode transitions.

## 3. Create and push the repository

Create an **empty** GitHub repository named `RLCD-DualMode-DisplayClock`, then run:

```powershell
$repo = 'E:\weixue\RLCD-DualMode-DisplayClock'
$remote = 'https://github.com/GITHUB_ACCOUNT/RLCD-DualMode-DisplayClock.git'

git -C $repo remote add origin $remote
git -C $repo branch -M main
git -C $repo push -u origin main
```

Verify the public repository page contains the root README, source, notices, and `docs/LOCAL_TEST_AND_RELEASE.md`, but no local generated outputs.

## 4. Release process

1. Build with ESP-IDF **v5.5.3** and complete the physical checklist.
2. Create a version tag only from the reviewed `main` commit.
3. Attach firmware binaries only after the exact build inputs and SHA-256 values have been recorded.
4. Include release notes covering: supported hardware, Windows driver version, panel resolution/format, the BOOT+KEY 1.5-second mode switch, known limitations, and rollback procedure.
5. Keep the Windows test-signed driver installer as a separate test artifact; do not imply that it is a production-signed driver.

Example tag and push:

```powershell
$repo = 'E:\weixue\RLCD-DualMode-DisplayClock'
git -C $repo tag -a v0.1.0 -m 'RLCD DualMode DisplayClock initial hardware-tested release'
git -C $repo push origin v0.1.0
```
