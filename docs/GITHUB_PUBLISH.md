# Publishing to GitHub

## Before the first push

1. Review `git status` and ensure that `main/wifi_secrets.h`, build outputs,
   device logs, generated binaries, certificates, and local Windows driver
   packages are absent.
2. Keep `LICENSE`, `NOTICE.md`, `docs/UPSTREAM.md`, and
   `firmware-clock/THIRD_PARTY_NOTICES.md` in the repository.
3. Create an empty GitHub repository named `RLCD-DualMode-DisplayClock`.

## Connect and push

```powershell
git -C E:\weixue\RLCD-DualMode-DisplayClock remote add origin GITHUB_REPOSITORY_URL
git -C E:\weixue\RLCD-DualMode-DisplayClock push -u origin main
```

## Release policy

Do not publish Wi-Fi credentials, API keys, NVS images, signed Windows driver
certificates, test certificates, or device-specific serial logs. Build artifacts
are published only after the combined firmware has completed hardware regression.
