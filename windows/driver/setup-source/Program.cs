using System.Diagnostics;
using System.Reflection;
using System.Text;

namespace RLCDDriverSetup;

internal static class Program
{
    [STAThread]
    private static void Main()
    {
        ApplicationConfiguration.Initialize();
        Application.Run(new SetupForm());
    }

    private sealed class SetupForm : Form
    {
        private readonly TextBox log = new() { Multiline = true, ReadOnly = true, ScrollBars = ScrollBars.Vertical, Dock = DockStyle.Fill, Font = new Font("Consolas", 9) };
        private readonly Button install = new() { Text = "安装 / 更新 RLCD 驱动", Dock = DockStyle.Bottom, Height = 44 };

        public SetupForm()
        {
            Text = "RLCD USB Display Driver Setup 1.2.25.7";
            ClientSize = new Size(720, 420);
            Controls.Add(log); Controls.Add(install);
            install.Click += async (_, _) => await InstallAsync();
            Add("已获得管理员权限。点击下方按钮开始安装 RLCD Mono1 驱动。");
        }

        private void Add(string value) { log.AppendText($"[{DateTime.Now:HH:mm:ss}] {value}{Environment.NewLine}"); }

        private async Task InstallAsync()
        {
            install.Enabled = false;
            var folder = Path.Combine(Path.GetTempPath(), "RLCD-Driver-1.2.25.7-" + Guid.NewGuid().ToString("N"));
            try
            {
                Directory.CreateDirectory(folder);
                Extract("Package.xfz1986_usb_graphic.dll", Path.Combine(folder, "xfz1986_usb_graphic.dll"));
                Extract("Package.xfz1986_usb_graphic.inf", Path.Combine(folder, "xfz1986_usb_graphic.inf"));
                Add("已释放驱动包。");

                var certScript = "$c=Get-ChildItem 'Cert:\\LocalMachine\\My' | Where-Object Subject -eq 'CN=RLCD Mono1 Display Test' | Select-Object -First 1; if($null -eq $c){$c=New-SelfSignedCertificate -Type CodeSigningCert -Subject 'CN=RLCD Mono1 Display Test' -CertStoreLocation 'Cert:\\LocalMachine\\My' -KeyExportPolicy Exportable -KeyUsage DigitalSignature -HashAlgorithm SHA256}; $c.Thumbprint";
                var thumb = (await Run("powershell.exe", $"-NoProfile -ExecutionPolicy Bypass -Command \"{certScript}\"", true)).Trim().Split(new[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries).Last();
                if (thumb.Length < 20) throw new InvalidOperationException("签名证书创建失败。" );
                var cer = Path.Combine(folder, "RLCD-Mono1-Test.cer");
                await Run("powershell.exe", $"-NoProfile -Command \"Export-Certificate -Cert ('Cert:\\LocalMachine\\My\\{thumb}') -FilePath '{cer}' -Force | Out-Null\"", true);
                await Run("certutil.exe", $"-addstore -f Root \"{cer}\"", true);
                await Run("certutil.exe", $"-addstore -f TrustedPublisher \"{cer}\"", true);
                Add("测试签名证书已加入受信任存储。");
                var configDirectory = @"C:\ProgramData\RLCD-USB-Display";
                Directory.CreateDirectory(configDirectory);
                await Run("icacls.exe", $"\"{configDirectory}\" /grant *S-1-5-32-545:(OI)(CI)M /T /C", true);
                Add("上位机配置目录已授予 Users 修改权限。");

                var inf2cat = FindTool("Inf2Cat.exe");
                var signtool = FindTool("signtool.exe");
                await Run(inf2cat, $"/driver:\"{folder}\" /os:10_X64", true);
                var cat = Path.Combine(folder, "xfz1986_usb_graphic.cat");
                await Run(signtool, $"sign /fd SHA256 /sha1 {thumb} /s My /sm \"{cat}\"", true);
                await Run("pnputil.exe", $"/add-driver \"{Path.Combine(folder, "xfz1986_usb_graphic.inf")}\" /install", true, true);
                Add("完成。请重新插拔设备；随后打开 RLCD-Control.exe 设置模式和帧率。");
                MessageBox.Show(this, "驱动 1.2.25.7 已安装完成。", "RLCD Driver Setup", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                Add("失败：" + ex.Message);
                MessageBox.Show(this, ex.Message, "RLCD Driver Setup", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally
            {
                install.Enabled = true;
                try { Directory.Delete(folder, true); } catch { }
            }
        }

        private static void Extract(string name, string destination)
        {
            using var input = Assembly.GetExecutingAssembly().GetManifestResourceStream(name) ?? throw new FileNotFoundException(name);
            using var output = File.Create(destination); input.CopyTo(output);
        }

        private async Task<string> Run(string file, string args, bool failOnError, bool acceptDeviceStateCode = false)
        {
            Add($"> {Path.GetFileName(file)} {args}");
            using var p = Process.Start(new ProcessStartInfo(file, args) { UseShellExecute = false, RedirectStandardOutput = true, RedirectStandardError = true, CreateNoWindow = true }) ?? throw new InvalidOperationException("进程启动失败：" + file);
            var output = await p.StandardOutput.ReadToEndAsync(); var error = await p.StandardError.ReadToEndAsync(); await p.WaitForExitAsync();
            if (!string.IsNullOrWhiteSpace(output)) Add(output.Trim());
            if (!string.IsNullOrWhiteSpace(error)) Add(error.Trim());
            if (failOnError && p.ExitCode != 0 && !(acceptDeviceStateCode && p.ExitCode == 259)) throw new InvalidOperationException($"{Path.GetFileName(file)} 返回 {p.ExitCode}。");
            if (acceptDeviceStateCode && p.ExitCode == 259) Add("PnP 返回 259：设备状态枚举已结束；驱动包已写入驱动存储。");
            return output;
        }

        private static string FindTool(string name)
        {
            var kits = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86), "Windows Kits", "10", "bin");
            if (Directory.Exists(kits))
            {
                var candidate = Directory.EnumerateFiles(kits, name, SearchOption.AllDirectories)
                    .Where(x => string.Equals(new DirectoryInfo(Path.GetDirectoryName(x)!).Name, "x64", StringComparison.OrdinalIgnoreCase))
                    .OrderByDescending(x => x, StringComparer.OrdinalIgnoreCase)
                    .FirstOrDefault();
                if (candidate is not null) return candidate;
            }
            throw new FileNotFoundException($"未找到 {name}。请安装 Windows 10/11 SDK 与 WDK。" );
        }
    }
}
