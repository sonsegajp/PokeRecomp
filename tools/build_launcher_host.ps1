param([string]$WebViewLibrary = "", [string]$OutputDirectory = "")
$ErrorActionPreference = "Stop"
$projectDir = Split-Path -Parent $PSScriptRoot
if ($WebViewLibrary -eq "") {
    $pythonCommand = (Get-Command python.exe -ErrorAction Stop).Source
    $WebViewLibrary = & $pythonCommand -c "import pathlib,webview;print(pathlib.Path(webview.__file__).parent / 'lib')"
    if ($LASTEXITCODE -ne 0) { throw 'Install pywebview, or specify -WebViewLibrary pointing to the WebView2 SDK DLL directory.' }
    $WebViewLibrary = $WebViewLibrary.Trim()
}
if ($OutputDirectory -eq "") { $OutputDirectory = Join-Path $projectDir 'dist\launcher' }
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
$core = Join-Path $WebViewLibrary 'Microsoft.Web.WebView2.Core.dll'
$forms = Join-Path $WebViewLibrary 'Microsoft.Web.WebView2.WinForms.dll'
$loader = Join-Path $WebViewLibrary 'runtimes\win-x64\native\WebView2Loader.dll'
foreach ($required in @($compiler,$core,$forms,$loader)) {
    if (-not (Test-Path -LiteralPath $required)) { throw "Required build input missing: $required" }
}
$outputExe = Join-Path $OutputDirectory 'PokeRecomp.exe'
$source = Join-Path $projectDir 'launcher\native\LauncherHost.cs'
& $compiler /nologo /target:winexe /platform:x64 /optimize+ "/out:$outputExe" /reference:System.dll /reference:System.Core.dll /reference:System.Drawing.dll /reference:System.Windows.Forms.dll /reference:System.Web.Extensions.dll "/reference:$core" "/reference:$forms" $source
if ($LASTEXITCODE -ne 0) { throw 'Native launcher build failed.' }
Copy-Item -LiteralPath $core -Destination $OutputDirectory -Force
Copy-Item -LiteralPath $forms -Destination $OutputDirectory -Force
Copy-Item -LiteralPath $loader -Destination $OutputDirectory -Force
@"
<?xml version="1.0" encoding="utf-8"?>
<configuration><startup useLegacyV2RuntimeActivationPolicy="true"><supportedRuntime version="v4.0" sku=".NETFramework,Version=v4.7.2"/></startup></configuration>
"@ | Set-Content -LiteralPath ($outputExe + '.config') -Encoding UTF8
$pythonWindowless = Get-Command pythonw.exe -ErrorAction SilentlyContinue
if ($pythonWindowless) { Set-Content -LiteralPath (Join-Path $OutputDirectory 'python-path.txt') -Value $pythonWindowless.Source -Encoding ASCII }
Write-Output "Built $outputExe"
