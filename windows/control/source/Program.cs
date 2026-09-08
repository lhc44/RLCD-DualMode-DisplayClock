using System.Diagnostics;
using System.Drawing;
using System.Text;

namespace RLCDControl;

internal static class Program
{
    private const string ConfigDirectory = @"C:\ProgramData\RLCD-USB-Display";
    private const string ConfigPath = ConfigDirectory + @"\mode.ini";
    private static readonly string[] Modes = ["Clear", "Dark", "Light", "Photo", "Invert", "InvertPhoto"];
    private static readonly int[] FpsValues = [10, 15, 20, 24, 30, 40, 50, 60];

    [STAThread]
    private static void Main()
    {
        ApplicationConfiguration.Initialize();
        Application.Run(new ControlForm());
    }

    private sealed class ControlForm : Form
    {
        private readonly ComboBox mode = new() { DropDownStyle = ComboBoxStyle.DropDownList };
        private readonly ComboBox fps = new() { DropDownStyle = ComboBoxStyle.DropDownList };
        private readonly Label status = new() { AutoSize = false, Height = 56, ForeColor = Color.FromArgb(35, 80, 35) };

        public ControlForm()
        {
            Text = "RLCD USB Display Control";
            Font = new Font("Microsoft YaHei UI", 10);
            ClientSize = new Size(438, 270);
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            StartPosition = FormStartPosition.CenterScreen;

            mode.Items.AddRange(Modes);
            fps.Items.AddRange(FpsValues.Cast<object>().ToArray());
            Controls.AddRange([
                LabelOf("显示模式", 26, 28), mode,
                LabelOf("目标帧率", 26, 88), fps,
                status,
                ButtonOf("应用到副屏", 26, 158, Apply),
                ButtonOf("打开 Windows 显示设置", 206, 158, (_, _) => OpenDisplaySettings()),
                ButtonOf("重新读取当前配置", 26, 210, (_, _) => LoadConfig())
            ]);
            mode.SetBounds(140, 23, 262, 34);
            fps.SetBounds(140, 83, 262, 34);
            status.SetBounds(26, 211, 376, 44);
            LoadConfig();
        }

        private static Label LabelOf(string text, int x, int y) => new() { Text = text, AutoSize = true, Location = new Point(x, y + 5) };
        private static Button ButtonOf(string text, int x, int y, EventHandler action)
        {
            var b = new Button { Text = text, Location = new Point(x, y), Size = new Size(166, 36) };
            b.Click += action;
            return b;
        }

        private void LoadConfig()
        {
            var selectedMode = "Clear";
            var selectedFps = 60;
            if (File.Exists(ConfigPath))
            {
                foreach (var line in File.ReadAllLines(ConfigPath, Encoding.ASCII))
                {
                    var pair = line.Split('=', 2);
                    if (pair.Length != 2) continue;
                    if (pair[0].Equals("Name", StringComparison.OrdinalIgnoreCase) && Modes.Contains(pair[1])) selectedMode = pair[1];
                    if (pair[0].Equals("Fps", StringComparison.OrdinalIgnoreCase) && int.TryParse(pair[1], out var value) && FpsValues.Contains(value)) selectedFps = value;
                }
            }
            mode.SelectedItem = selectedMode;
            fps.SelectedItem = selectedFps;
            status.Text = File.Exists(ConfigPath) ? "已读取当前模式；选择后点击“应用到副屏”。" : "未发现配置，当前将使用 Clear / 60 FPS。";
        }

        private void Apply(object? sender, EventArgs e)
        {
            var name = mode.SelectedItem?.ToString() ?? "Clear";
            var fpsValue = (int)(fps.SelectedItem ?? 60);
            var index = Array.IndexOf(Modes, name);
            var lines = new[] { "[RLCD]", $"Mode={index}", $"Name={name}", $"Fps={fpsValue}" };
            try
            {
                Directory.CreateDirectory(ConfigDirectory);
                File.WriteAllLines(ConfigPath, lines, Encoding.ASCII);
            }
            catch (UnauthorizedAccessException)
            {
                try
                {
                    status.Text = "正在请求一次管理员权限，修复上位机配置文件权限…";
                    Application.DoEvents();
                    var fix = Process.Start(new ProcessStartInfo("icacls.exe", $"\"{ConfigDirectory}\" /grant *S-1-5-32-545:(OI)(CI)M /T /C") { UseShellExecute = true, Verb = "runas" });
                    fix?.WaitForExit();
                    File.WriteAllLines(ConfigPath, lines, Encoding.ASCII);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(this, "修复副屏配置权限失败：" + ex.Message, "RLCD Control", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(this, "写入副屏配置失败：" + ex.Message, "RLCD Control", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            status.Text = $"已应用：{name}，{fpsValue} FPS。驱动将在下一次画面刷新时生效。";
        }

        private static void OpenDisplaySettings()
        {
            Process.Start(new ProcessStartInfo("ms-settings:display") { UseShellExecute = true });
        }
    }
}
