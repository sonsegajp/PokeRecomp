using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Web.Script.Serialization;
using System.Windows.Forms;
using Microsoft.Web.WebView2.Core;
using Microsoft.Web.WebView2.WinForms;

[assembly: System.Reflection.AssemblyTitle("Pokemon Recompiled GEN 3")]
[assembly: System.Reflection.AssemblyProduct("Pokemon Recompiled Launcher")]
[assembly: System.Reflection.AssemblyVersion("1.0.0.0")]

internal static class Program
{
    [STAThread]
    private static void Main(string[] args)
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        string root = Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "..", ".."));
        string dataDirectory = null;
        int debugPort = 0;
        for (int i = 0; i < args.Length; i++)
        {
            if (args[i] == "--root" && i + 1 < args.Length) root = Path.GetFullPath(args[++i]);
            else if (args[i] == "--data-dir" && i + 1 < args.Length) dataDirectory = Path.GetFullPath(args[++i]);
            else if (args[i] == "--debug-port" && i + 1 < args.Length) debugPort = int.Parse(args[++i]);
        }
        if (debugPort < 0 || debugPort > 65535) throw new ArgumentOutOfRangeException("debug-port");
        Application.Run(new LauncherWindow(root, dataDirectory, debugPort));
    }
}

internal sealed class LocalWebClient : WebClient
{
    protected override WebRequest GetWebRequest(Uri address)
    {
        WebRequest request = base.GetWebRequest(address);
        request.Timeout = 2500;
        return request;
    }
}

// The panel the game's OpenGL window is embedded in. It must never paint over
// its child: no background erase (WS_CLIPCHILDREN, opaque), or the game's
// frame flickers every other frame while the panel repaints underneath.
internal sealed class GamePanel : Panel
{
    public GamePanel()
    {
        SetStyle(ControlStyles.UserPaint | ControlStyles.AllPaintingInWmPaint | ControlStyles.Opaque, true);
    }

    protected override CreateParams CreateParams
    {
        get { CreateParams cp = base.CreateParams; cp.Style |= 0x02000000; return cp; }   // WS_CLIPCHILDREN
    }

    protected override void OnPaintBackground(PaintEventArgs e)
    {
        if (!HasChildWindow()) base.OnPaintBackground(e);
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        if (!HasChildWindow()) e.Graphics.Clear(BackColor);
    }

    [System.Runtime.InteropServices.DllImport("user32.dll")] private static extern IntPtr GetWindow(IntPtr window, uint command);
    private bool HasChildWindow() { return IsHandleCreated && GetWindow(Handle, 5 /* GW_CHILD */) != IntPtr.Zero; }
}

internal sealed class LauncherWindow : Form
{
    private readonly string root;
    private readonly string dataDirectory;
    private readonly int debugPort;
    private readonly string stateDirectory;
    private readonly JavaScriptSerializer json = new JavaScriptSerializer();
    private readonly WebView2 browser = new WebView2();
    private readonly GamePanel gamePanel = new GamePanel();
    private readonly Label loading = new Label();
    private readonly System.Windows.Forms.Timer pollTimer = new System.Windows.Forms.Timer();
    private readonly System.Windows.Forms.Timer keyTimer = new System.Windows.Forms.Timer();
    private Process serverProcess;
    private string baseUrl;
    private string token;
    private int gamePid;
    private string gameMode;
    private IntPtr gameWindow;
    private bool polling;
    private bool closing;
    private bool allowClose;
    private bool quittingFromWeb;
    private bool fullscreen;
    private bool escapeHeld;
    private bool f11Held;
    private bool exitPrompt;
    private bool togglingFullscreen;
    private Rectangle windowedBounds;
    private FormWindowState windowedState;

    [DllImport("user32.dll")] private static extern bool EnumWindows(EnumWindowProc callback, IntPtr parameter);
    [DllImport("user32.dll")] private static extern bool EnumChildWindows(IntPtr parent, EnumWindowProc callback, IntPtr parameter);
    [DllImport("user32.dll")] private static extern uint GetWindowThreadProcessId(IntPtr window, out uint processId);
    [DllImport("user32.dll")] private static extern IntPtr GetParent(IntPtr window);
    [DllImport("user32.dll")] private static extern IntPtr SetParent(IntPtr child, IntPtr parent);
    [DllImport("user32.dll", EntryPoint = "GetWindowLongPtrW")] private static extern IntPtr GetWindowLongPtr(IntPtr window, int index);
    [DllImport("user32.dll", EntryPoint = "SetWindowLongPtrW")] private static extern IntPtr SetWindowLongPtr(IntPtr window, int index, IntPtr value);
    [DllImport("user32.dll")] private static extern bool SetWindowPos(IntPtr window, IntPtr after, int x, int y, int width, int height, uint flags);
    [DllImport("user32.dll")] private static extern bool IsWindowVisible(IntPtr window);
    [DllImport("user32.dll")] private static extern bool IsWindow(IntPtr window);
    [DllImport("user32.dll")] private static extern bool GetWindowRect(IntPtr window, out Rect rectangle);
    [DllImport("user32.dll")] private static extern IntPtr SetFocus(IntPtr window);
    [DllImport("user32.dll")] private static extern bool PostMessage(IntPtr window, uint message, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll")] private static extern short GetAsyncKeyState(int key);
    [DllImport("user32.dll")] private static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")] private static extern IntPtr GetAncestor(IntPtr window, uint flags);
    [DllImport("user32.dll")] private static extern bool GetClientRect(IntPtr window, out Rect rectangle);
    private delegate bool EnumWindowProc(IntPtr window, IntPtr parameter);
    [StructLayout(LayoutKind.Sequential)] private struct Rect { public int Left, Top, Right, Bottom; }

    public LauncherWindow(string rootDirectory, string saveDirectory, int remoteDebugPort)
    {
        root = rootDirectory;
        dataDirectory = saveDirectory;
        debugPort = remoteDebugPort;
        stateDirectory = Path.Combine(root, "launcher", "state", "native-" + Process.GetCurrentProcess().Id);
        Directory.CreateDirectory(stateDirectory);
        Text = "Pokémon Recompiled · GEN 3";
        ClientSize = new Size(1280, 720);
        MinimumSize = new Size(960, 600);
        StartPosition = FormStartPosition.CenterScreen;
        BackColor = Color.Black;
        KeyPreview = true;
        SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.Opaque, true);
        browser.Dock = DockStyle.Fill;
        browser.DefaultBackgroundColor = Color.Black;
        gamePanel.Dock = DockStyle.Fill;
        gamePanel.BackColor = Color.Black;
        gamePanel.Visible = false;
        gamePanel.TabStop = true;
        loading.Dock = DockStyle.Fill;
        loading.TextAlign = ContentAlignment.MiddleCenter;
        loading.ForeColor = Color.White;
        loading.Font = new Font("Segoe UI", 18);
        loading.Text = "Opening Pokémon Recompiled…";
        Controls.Add(browser);
        Controls.Add(gamePanel);
        Controls.Add(loading);
        gamePanel.Resize += delegate { SizeGameWindow(); };
        gamePanel.Click += delegate { FocusGameWindow(); };
        // After Alt+Tab, WinForms restores focus to its own last control once Activated has run;
        // handing the keyboard to the game afterwards (BeginInvoke) keeps it with the game.
        Activated += delegate { if (gamePid != 0 && !exitPrompt) BeginInvoke(new Action(FocusGameWindow)); };
        pollTimer.Interval = 400;
        pollTimer.Tick += async delegate { await PollState(); };
        keyTimer.Interval = 40;
        keyTimer.Tick += async delegate { await CheckHostKeys(); };
        Shown += async delegate { await Initialize(); };
        FormClosing += OnClosing;
    }

    private void Log(string message)
    {
        try { File.AppendAllText(Path.Combine(stateDirectory, "host.log"), DateTime.Now.ToString("s") + " " + message + Environment.NewLine); }
        catch { }
    }

    private async Task Initialize()
    {
        try
        {
            // Create the stable native viewport handle before the helper starts.
            IntPtr parentHandle = gamePanel.Handle;
            string python = FindPython();
            List<string> arguments = new List<string>();
            arguments.Add(Path.Combine(root, "launcher", "server.py"));
            arguments.Add("--state-dir"); arguments.Add(stateDirectory);
            arguments.Add("--library-dir"); arguments.Add(dataDirectory == null ? Path.Combine(root, "launcher", "state", "library") : Path.Combine(dataDirectory, ".launcher-library"));
            arguments.Add("--embed-parent"); arguments.Add(parentHandle.ToInt64().ToString());
            if (dataDirectory != null) { arguments.Add("--data-dir"); arguments.Add(dataDirectory); }
            ProcessStartInfo start = new ProcessStartInfo(python, JoinArguments(arguments));
            start.WorkingDirectory = root;
            start.UseShellExecute = false;
            start.CreateNoWindow = true;
            start.WindowStyle = ProcessWindowStyle.Hidden;
            serverProcess = Process.Start(start);
            string sessionPath = Path.Combine(stateDirectory, "session.json");
            for (int attempt = 0; attempt < 150 && !closing; attempt++)
            {
                if (serverProcess.HasExited) throw new Exception("The local launcher service stopped. Run python launcher/server.py to inspect the error.");
                if (File.Exists(sessionPath))
                {
                    Dictionary<string, object> session = json.Deserialize<Dictionary<string, object>>(File.ReadAllText(sessionPath));
                    baseUrl = Convert.ToString(session["url"]);
                    Uri address = new Uri(baseUrl);
                    if (address.Scheme != "http" || address.Host != "127.0.0.1") throw new Exception("The launcher service reported an invalid address.");
                    break;
                }
                await Task.Delay(100);
            }
            if (closing) return;
            if (baseUrl == null) throw new Exception("The launcher service did not become ready.");
            Dictionary<string, object> state = await FetchState();
            token = Convert.ToString(state["token"]);
            ApplyFullscreen(state);
            CoreWebView2EnvironmentOptions options = new CoreWebView2EnvironmentOptions();
            options.AdditionalBrowserArguments = "--autoplay-policy=no-user-gesture-required";
            if (debugPort != 0) options.AdditionalBrowserArguments += " --remote-debugging-port=" + debugPort + " --remote-debugging-address=127.0.0.1";
            string browserData = Path.Combine(root, "launcher", "state", debugPort == 0 ? "webview" : "webview-debug-" + debugPort);
            CoreWebView2Environment environment = await CoreWebView2Environment.CreateAsync(null, browserData, options);
            await browser.EnsureCoreWebView2Async(environment);
            browser.CoreWebView2.Settings.AreDefaultContextMenusEnabled = false;
            browser.CoreWebView2.Settings.AreDevToolsEnabled = debugPort != 0;
            browser.CoreWebView2.Settings.IsStatusBarEnabled = false;
            browser.CoreWebView2.Settings.IsZoomControlEnabled = false;
            browser.CoreWebView2.Settings.IsWebMessageEnabled = true;
            browser.CoreWebView2.NavigationStarting += delegate(object sender, CoreWebView2NavigationStartingEventArgs args)
            {
                Uri destination;
                if (!Uri.TryCreate(args.Uri, UriKind.Absolute, out destination) ||
                    (args.Uri != "about:blank" && destination.GetLeftPart(UriPartial.Authority) != baseUrl)) args.Cancel = true;
            };
            browser.CoreWebView2.NewWindowRequested += delegate(object sender, CoreWebView2NewWindowRequestedEventArgs args) { args.Handled = true; };
            browser.CoreWebView2.PermissionRequested += delegate(object sender, CoreWebView2PermissionRequestedEventArgs args) { args.State = CoreWebView2PermissionState.Deny; };
            browser.CoreWebView2.WebMessageReceived += delegate(object sender, CoreWebView2WebMessageReceivedEventArgs args)
            {
                try
                {
                    Dictionary<string, object> message = json.Deserialize<Dictionary<string, object>>(args.WebMessageAsJson);
                    object type;
                    if (message.TryGetValue("type", out type) && Convert.ToString(type) == "quit")
                    {
                        quittingFromWeb = true;
                        Close();
                    }
                }
                catch (Exception error) { Log(error.Message); }
            };
            browser.CoreWebView2.NavigationCompleted += delegate { loading.Visible = false; };
            browser.Source = new Uri(baseUrl);
            pollTimer.Start();
            keyTimer.Start();
            Log("Ready " + baseUrl + " parent HWND " + parentHandle);
        }
        catch (Exception error)
        {
            Log(error.ToString());
            loading.Text = "Pokémon Recompiled could not start.\n\n" + error.Message + "\n\nSee " + Path.Combine(stateDirectory, "host.log");
            MessageBox.Show(this, error.Message + "\n\nThe Microsoft Edge WebView2 Runtime and Python 3.10 or newer are required.", "Pokémon Recompiled", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }

    private static string FindPython()
    {
        string hint = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "python-path.txt");
        if (File.Exists(hint) && File.Exists(File.ReadAllText(hint).Trim())) return File.ReadAllText(hint).Trim();
        foreach (string name in new string[] { "pythonw.exe", "python.exe" })
            foreach (string directory in (Environment.GetEnvironmentVariable("PATH") ?? "").Split(Path.PathSeparator))
            {
                try { string path = Path.Combine(directory.Trim('"'), name); if (File.Exists(path) && path.IndexOf("WindowsApps", StringComparison.OrdinalIgnoreCase) < 0) return path; }
                catch { }
            }
        string installs = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Programs", "Python");
        if (Directory.Exists(installs))
            foreach (string directory in Directory.GetDirectories(installs))
            {
                string path = Path.Combine(directory, "pythonw.exe");
                if (File.Exists(path)) return path;
            }
        throw new FileNotFoundException("Python was not found. Install Python 3.10 or newer and launch again.");
    }

    private static string JoinArguments(List<string> arguments)
    {
        List<string> quoted = new List<string>();
        foreach (string argument in arguments)
        {
            StringBuilder result = new StringBuilder("\"");
            int slashes = 0;
            foreach (char character in argument)
            {
                if (character == '\\') { slashes++; continue; }
                if (character == '"') { result.Append('\\', slashes * 2 + 1); result.Append('"'); slashes = 0; continue; }
                result.Append('\\', slashes); slashes = 0; result.Append(character);
            }
            result.Append('\\', slashes * 2); result.Append('"'); quoted.Add(result.ToString());
        }
        return string.Join(" ", quoted.ToArray());
    }

    private async Task<Dictionary<string, object>> FetchState()
    {
        using (LocalWebClient client = new LocalWebClient())
        {
            client.Encoding = Encoding.UTF8;
            string text = await client.DownloadStringTaskAsync(new Uri(baseUrl + "/api/state"));
            return json.Deserialize<Dictionary<string, object>>(text);
        }
    }

    private async Task PollState()
    {
        if (polling || closing || baseUrl == null) return;
        polling = true;
        try
        {
            Dictionary<string, object> state = await FetchState();
            if (!togglingFullscreen) ApplyFullscreen(state);
            object value;
            Dictionary<string, object> running = state.TryGetValue("running", out value) ? value as Dictionary<string, object> : null;
            if (running != null)
            {
                // Adopt a soft-reset replacement only if it is already our child.
                object external;
                int pid = Convert.ToInt32(running["pid"]);
                if (running.TryGetValue("external", out external) && Convert.ToBoolean(external))
                {
                    IntPtr replacement = FindGameWindow(pid);
                    if (replacement == IntPtr.Zero || GetParent(replacement) != gamePanel.Handle)
                    {
                        if (gamePid != 0 && !GameIsAlive()) ShowLauncher();
                        return;
                    }
                }
                string mode = Convert.ToString(running["mode"]);
                bool changed = pid != gamePid;
                gamePid = pid;
                gameMode = mode;
                // IsWindowVisible(child) is false while its parent panel is hidden.
                gamePanel.Visible = true;
                bool needsFocus = changed || gameWindow == IntPtr.Zero;
                if (changed || gameWindow == IntPtr.Zero || !IsWindow(gameWindow)) gameWindow = FindGameOrLegWindow(pid);
                if (gameWindow != IntPtr.Zero)
                {
                    if (GetParent(gameWindow) != gamePanel.Handle) EmbedGameWindow();
                    if (browser.Visible || loading.Visible || changed) { browser.Visible = false; loading.Visible = false; gamePanel.Visible = true; gamePanel.BringToFront(); SizeGameWindow(); }
                    if (needsFocus) FocusGameWindow();
                }
            }
            else if (gamePid != 0)
            {
                ShowLauncher();
            }
        }
        catch (Exception error) { Log("Poll: " + error.Message); }
        finally { polling = false; }
    }

    private void ShowLauncher()
    {
        gamePid = 0; gameWindow = IntPtr.Zero; gameMode = null;
        gamePanel.Visible = false;
        browser.Visible = true; browser.BringToFront(); browser.Focus();
        NotifyLauncherReturned();
    }

    private async void NotifyLauncherReturned()
    {
        try
        {
            if (closing || browser.CoreWebView2 == null) return;
            await browser.CoreWebView2.ExecuteScriptAsync("window.dispatchEvent(new Event('launcher-game-return'));");
            if (!closing && gamePid == 0) browser.Focus();
            Log("Launcher input restored after game exit");
        }
        catch (Exception error) { Log("Launcher return: " + error.Message); }
    }

    // The game's window, or on a trip between regions the current leg's: that
    // game runs in a child process of the one we started (runner travel_leg)
    // and puts its window in our game panel itself.
    private IntPtr FindGameOrLegWindow(int pid)
    {
        IntPtr found = FindGameWindow(pid);
        if (found != IntPtr.Zero) return found;
        EnumWindowProc callback = delegate(IntPtr window, IntPtr parameter)
        {
            Rect bounds;
            if (GetParent(window) == gamePanel.Handle && IsWindowVisible(window) && GetWindowRect(window, out bounds)
             && bounds.Right - bounds.Left > 64 && bounds.Bottom - bounds.Top > 64) { found = window; return false; }
            return true;
        };
        EnumChildWindows(gamePanel.Handle, callback, IntPtr.Zero);
        return found;
    }

    private IntPtr FindGameWindow(int pid)
    {
        IntPtr found = IntPtr.Zero;
        EnumWindowProc callback = delegate(IntPtr window, IntPtr parameter)
        {
            uint owner;
            GetWindowThreadProcessId(window, out owner);
            if (owner == pid && IsWindowVisible(window))
            {
                Rect bounds;
                if (GetWindowRect(window, out bounds) && bounds.Right - bounds.Left > 64 && bounds.Bottom - bounds.Top > 64) { found = window; return false; }
            }
            return true;
        };
        EnumChildWindows(gamePanel.Handle, callback, IntPtr.Zero);
        if (found == IntPtr.Zero) EnumWindows(callback, IntPtr.Zero);
        return found;
    }

    private void EmbedGameWindow()
    {
        long style = GetWindowLongPtr(gameWindow, -16).ToInt64();
        style = (style & ~0x80CF0000L) | 0x50000000L;
        SetWindowLongPtr(gameWindow, -16, new IntPtr(style));
        SetParent(gameWindow, gamePanel.Handle);
        SizeGameWindow();
    }

    private void SizeGameWindow()
    {
        if (gameWindow != IntPtr.Zero && IsWindow(gameWindow))
        {
            int width = Math.Max(1, gamePanel.ClientSize.Width), height = Math.Max(1, gamePanel.ClientSize.Height);
            Rect existing;
            if (!GetClientRect(gameWindow, out existing) || existing.Right != width || existing.Bottom != height)
                SetWindowPos(gameWindow, IntPtr.Zero, 0, 0, width, height, 0x0014);
        }
    }

    protected override void WndProc(ref Message message)
    {
        if (message.Msg == 0x8031 && gamePid != 0 && !exitPrompt && !closing)
        {
            BeginInvoke(new Action(async delegate { await RequestGameClose(false); }));
            return;
        }
        base.WndProc(ref message);
    }

    private void ApplyFullscreen(Dictionary<string, object> state)
    {
        object value;
        Dictionary<string, object> settings = state.TryGetValue("settings", out value) ? value as Dictionary<string, object> : null;
        bool enabled = settings != null && settings.TryGetValue("fullscreen", out value) && Convert.ToInt32(value) != 0;
        if (enabled == fullscreen) return;
        fullscreen = enabled;
        SuspendLayout();
        if (enabled)
        {
            windowedState = WindowState;
            windowedBounds = WindowState == FormWindowState.Normal ? Bounds : RestoreBounds;
            Rectangle monitor = Screen.FromControl(this).Bounds;
            WindowState = FormWindowState.Normal;
            FormBorderStyle = FormBorderStyle.None;
            Bounds = monitor;
        }
        else
        {
            FormBorderStyle = FormBorderStyle.Sizable;
            WindowState = FormWindowState.Normal;
            if (windowedBounds.Width > 0) Bounds = windowedBounds;
            WindowState = windowedState;
        }
        ResumeLayout(true);
        SizeGameWindow();
        Log("Fullscreen " + enabled);
    }

    private async Task CheckHostKeys()
    {
        IntPtr active = GetForegroundWindow();
        bool focused = active == Handle || GetAncestor(active, 2) == Handle;
        bool escape = (GetAsyncKeyState(0x1B) & 0x8000) != 0;
        bool f11 = (GetAsyncKeyState(0x7A) & 0x8000) != 0;
        bool exit = escape && !escapeHeld && focused && gamePid != 0 && !exitPrompt;
        bool toggle = f11 && !f11Held && focused && gamePid == 0 && !togglingFullscreen;
        escapeHeld = escape; f11Held = f11;
        if (exit) await RequestGameClose(false);
        if (toggle)
        {
            togglingFullscreen = true;
            try
            {
                using (LocalWebClient client = new LocalWebClient())
                {
                    client.Encoding = Encoding.UTF8;
                    client.Headers["Content-Type"] = "application/json";
                    client.Headers["X-Launcher-Token"] = token;
                    await client.UploadStringTaskAsync(new Uri(baseUrl + "/api/settings"), "POST", "{\"settings\":{\"fullscreen\":" + (fullscreen ? "0" : "1") + "}}");
                }
                ApplyFullscreen(await FetchState());
            }
            catch (Exception error) { Log("Fullscreen toggle: " + error.Message); }
            finally { togglingFullscreen = false; }
        }
    }

    private void FocusGameWindow()
    {
        if (gameWindow != IntPtr.Zero && IsWindow(gameWindow)) SetFocus(gameWindow);
    }

    private bool GameIsAlive()
    {
        if (gamePid == 0) return false;
        try { using (Process game = Process.GetProcessById(gamePid)) return !game.HasExited; }
        catch { return false; }
    }

    private async Task<bool> RequestGameClose(bool closeLauncher)
    {
        if (!GameIsAlive()) return true;
        if (exitPrompt) return false;
        exitPrompt = true;
        try
        {
        string message = gameMode == "game"
            ? "Return to the launcher? Any progress since your last in-game save will be lost."
            : gameMode == "gifts" ? "Close Mystery Gift and return to the launcher? Any unsaved changes will be lost."
            : "Discard unsaved PC Box changes and return? To save instead, choose No here, then press Z / B in the game and choose NO at Continue Box Operations.";
        if (MessageBox.Show(this, message, closeLauncher ? "Close Pokémon Recompiled?" : "Return to launcher?", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2) != DialogResult.Yes) { FocusGameWindow(); return false; }
        if (gameWindow == IntPtr.Zero || !IsWindow(gameWindow)) gameWindow = FindGameOrLegWindow(gamePid);
        if (gameWindow == IntPtr.Zero) { MessageBox.Show(this, "Close the game's window before closing the launcher.", "Game is still running"); return false; }
        PostMessage(gameWindow, 0x0010, IntPtr.Zero, IntPtr.Zero);
        for (int attempt = 0; attempt < 50 && GameIsAlive(); attempt++) await Task.Delay(100);
        if (GameIsAlive())
        {
            MessageBox.Show(this, "The game is still running. Finish its save or close prompt, then return to the launcher.", "Game is still running");
            return false;
        }
        return true;
        }
        finally { exitPrompt = false; }
    }

    private async void OnClosing(object sender, FormClosingEventArgs args)
    {
        if (allowClose) return;
        args.Cancel = true;
        if (closing) return;
        closing = true;
        try
        {
            if (baseUrl != null && !quittingFromWeb)
            {
                try
                {
                    Dictionary<string, object> state = await FetchState();
                    object value;
                    Dictionary<string, object> running = state.TryGetValue("running", out value) ? value as Dictionary<string, object> : null;
                    if (running != null)
                    {
                        object external;
                        int pid = Convert.ToInt32(running["pid"]);
                        IntPtr candidate = FindGameOrLegWindow(pid);
                        bool owned = !running.TryGetValue("external", out external) || !Convert.ToBoolean(external)
                            || (candidate != IntPtr.Zero && GetParent(candidate) == gamePanel.Handle);
                        if (owned) { gamePid = pid; gameMode = Convert.ToString(running["mode"]); gameWindow = candidate; }
                    }
                }
                catch { }
            }
            if (!await RequestGameClose(true)) { closing = false; return; }
            if (baseUrl != null && token != null && !quittingFromWeb)
            {
                try
                {
                    using (LocalWebClient client = new LocalWebClient())
                    {
                        client.Headers["Content-Type"] = "application/json";
                        client.Headers["X-Launcher-Token"] = token;
                        await client.UploadStringTaskAsync(new Uri(baseUrl + "/api/quit"), "POST", "{}");
                    }
                }
                catch (Exception error) { Log("Shutdown: " + error.Message); }
            }
            pollTimer.Stop();
            keyTimer.Stop();
            allowClose = true;
            browser.Dispose();
            Close();
        }
        catch (Exception error) { Log(error.ToString()); closing = false; }
    }
}
