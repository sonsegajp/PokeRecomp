param([string]$DataDir = "", [switch]$Browser)
$ErrorActionPreference = "Stop"
$projectDir = $PSScriptRoot
$launcherDir = Join-Path $projectDir "launcher"
$nativeHost = Join-Path $projectDir "dist\launcher\PokeRecomp.exe"
if ((Test-Path -LiteralPath $nativeHost) -and -not $Browser) {
    $nativeArguments = ''
    if ($DataDir -ne "") {
        if ($DataDir.Contains('"')) { throw "The save directory cannot contain a quote." }
        $nativeArguments = '--data-dir "' + [IO.Path]::GetFullPath($DataDir) + '"'
    }
    if ($nativeArguments) {
        Start-Process -FilePath $nativeHost -ArgumentList $nativeArguments -WorkingDirectory $projectDir -WindowStyle Normal
    } else {
        Start-Process -FilePath $nativeHost -WorkingDirectory $projectDir -WindowStyle Normal
    }
    return
}
$sessionPath = Join-Path $launcherDir "state\session.json"
$launcherUrl = $null
if (Test-Path -LiteralPath $sessionPath) {
    try {
        $session = Get-Content -LiteralPath $sessionPath -Raw | ConvertFrom-Json
        $sessionUri = [Uri]$session.url
        if ($sessionUri.Scheme -eq "http" -and $sessionUri.Host -eq "127.0.0.1" -and $sessionUri.Port -gt 0) {
            $existing = Invoke-RestMethod -Uri ($session.url + "/api/state") -TimeoutSec 2
            if ($existing.ok -and ($DataDir -eq "" -or $existing.dataDir -eq $DataDir)) { $launcherUrl = $session.url }
        }
    } catch { }
}
if (-not $launcherUrl) {
    $pythonPath = $null
    foreach ($commandName in @("pythonw.exe", "python.exe", "py.exe")) {
        $candidate = Get-Command $commandName -ErrorAction SilentlyContinue
        if ($candidate) { $pythonPath = $candidate.Source; break }
    }
    if ($pythonPath -and (Split-Path -Leaf $pythonPath) -eq "py.exe") {
        # Resolve the launcher shim to its interpreter so the readiness PID matches.
        $resolvedPython = & $pythonPath -3 -c "import sys; print(sys.executable)"
        if ($LASTEXITCODE -eq 0 -and $resolvedPython -and (Test-Path -LiteralPath $resolvedPython)) {
            $pythonPath = $resolvedPython.Trim()
            $windowlessPython = Join-Path (Split-Path -Parent $pythonPath) "pythonw.exe"
            if (Test-Path -LiteralPath $windowlessPython) { $pythonPath = $windowlessPython }
        } else { $pythonPath = $null }
    }
    if (-not $pythonPath) {
        throw "PokeRecomp needs Python 3.10 or newer. Install Python, then run this launcher again."
    }
    $serverPath = Join-Path $launcherDir "server.py"
    $arguments = '"' + $serverPath + '"'
    if ($DataDir -ne "") {
        if ($DataDir.Contains('"')) { throw "The save directory cannot contain a quote." }
        $arguments += ' --data-dir "' + [IO.Path]::GetFullPath($DataDir) + '"'
    }
    $helper = Start-Process -FilePath $pythonPath -ArgumentList $arguments -WorkingDirectory $projectDir -WindowStyle Hidden -PassThru
    for ($attempt = 0; $attempt -lt 80; $attempt++) {
        Start-Sleep -Milliseconds 100
        if ($helper.HasExited) { throw "The launcher service stopped. Run python launcher/server.py to see the error." }
        if (Test-Path -LiteralPath $sessionPath) {
            try {
                $session = Get-Content -LiteralPath $sessionPath -Raw | ConvertFrom-Json
                if ($session.pid -eq $helper.Id) {
                    $ready = Invoke-RestMethod -Uri ($session.url + "/api/state") -TimeoutSec 1
                    if ($ready.ok) { $launcherUrl = $session.url; break }
                }
            } catch { }
        }
    }
    if (-not $launcherUrl) { throw "The launcher service did not become ready. Run python launcher/server.py to see the error." }
}
$browserPath = $null
foreach ($candidate in @(
    (Join-Path ${env:ProgramFiles(x86)} "Microsoft\Edge\Application\msedge.exe"),
    (Join-Path $env:ProgramFiles "Microsoft\Edge\Application\msedge.exe"),
    (Join-Path $env:ProgramFiles "Google\Chrome\Application\chrome.exe"),
    (Join-Path $env:LOCALAPPDATA "Google\Chrome\Application\chrome.exe")
)) {
    if (Test-Path -LiteralPath $candidate) { $browserPath = $candidate; break }
}
if ($browserPath -and -not $Browser) {
    # The app is visible; the background helper remains hidden.
    $appArguments = '--app="' + $launcherUrl + '" --window-size=1440,960 --no-first-run'
    Start-Process -FilePath $browserPath -ArgumentList $appArguments -WindowStyle Normal
} else {
    Start-Process -FilePath $launcherUrl
}
