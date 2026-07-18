#requires -Version 5.1
# Launch-Inject.ps1 - minimal KDU + SCM + kernel-inject launcher.
# ASCII-only comments. UTF-8 BOM safe under Windows PowerShell 5.1.

[CmdletBinding()]
param(
    [string]$Dll,
    [string]$KduProvider = "1",   # 1 = RTCore64, 0 = Intel NAL
    [switch]$WithFallback
)

$ErrorActionPreference = 'Stop'
$dir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $dir

$driver   = Join-Path $dir "CS2HexSyncCompatDriver.sys"
$injector = Join-Path $dir "CS2UnifiedInjector.exe"
$kdu      = Join-Path $dir "kdu.exe"
$cer      = Join-Path $dir "CS2HexSyncCompatCert.cer"
$svcName  = "CS2HexSyncCompatSvc"

function Info($m) { Write-Host "[*] $m" -ForegroundColor Cyan }
function Ok  ($m) { Write-Host "[+] $m" -ForegroundColor Green }
function Warn($m) { Write-Host "[!] $m" -ForegroundColor Yellow }
function Die ($m) { Write-Host "[-] $m" -ForegroundColor Red; exit 1 }

# 1. Admin
$isAdmin = ([Security.Principal.WindowsPrincipal] `
    [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
    [Security.Principal.WindowsBuiltInRole] "Administrator")
if (-not $isAdmin) { Die "Run as Administrator." }

# 2. Files
foreach ($f in @($driver, $injector, $kdu, $cer)) {
    if (-not (Test-Path $f)) { Die "Missing: $(Split-Path $f -Leaf)" }
}

# 2b. Auto-close MSI Afterburner / RivaTuner (they hold RTCore64 exclusive).
# Without this, KDU -prv 1 (RTCore64) fails with "device open in use".
$mustStop = @('MSIAfterburner', 'RTSS', 'EncoderServer', 'HwInfo64', 'HwInfo32')
foreach ($p in $mustStop) {
    $proc = Get-Process $p -ErrorAction SilentlyContinue
    if ($proc) {
        try {
            $proc | Stop-Process -Force -ErrorAction Stop
            Ok "Stopped $p (holds RTCore64 exclusive)"
        } catch { Warn "Could not stop ${p}: $_" }
    }
}
Start-Sleep -Milliseconds 500

# 3. Win11 22H2+ vulnerable driver blocklist off (needed for RTCore64)
if ([Environment]::OSVersion.Version.Build -ge 22621) {
    $ci = "HKLM:\System\CurrentControlSet\Control\CI\Config"
    if (-not (Test-Path $ci)) { New-Item -Path $ci -Force | Out-Null }
    Set-ItemProperty -Path $ci -Name VulnerableDriverBlocklistEnable -Value 0 -Type DWord -Force
    Ok "MSFT driver blocklist disabled"
}

# 4. Wait for cs2.exe (60s cap)
Info "Waiting for cs2.exe..."
$cs2 = $null; $t = 0
while (-not $cs2 -and $t -lt 60) {
    $cs2 = Get-Process cs2 -ErrorAction SilentlyContinue
    if (-not $cs2) { Start-Sleep -Seconds 1; $t++ }
}
if (-not $cs2) { Die "cs2.exe not running (60s wait)." }
Ok "cs2.exe PID $($cs2.Id)"

# 5. Pick DLL (SHA of client.dll on disk -> depot; default = 24134959)
if ($Dll) {
    $dllPath = if ([IO.Path]::IsPathRooted($Dll)) { $Dll } else { Join-Path $dir $Dll }
} else {
    $paths = @(
        "D:\SteamLibrary\steamapps\common\Counter-Strike Global Offensive\game\csgo\bin\win64\client.dll",
        "C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\game\csgo\bin\win64\client.dll"
    )
    $client = $paths | Where-Object { Test-Path $_ } | Select-Object -First 1
    $chosen = "fva_recon_24134959.dll"
    if ($client) {
        $sha = (Get-FileHash $client -Algorithm SHA256).Hash
        if ($sha -eq "868E1DF4BA4A2A1E73C3A5A563405BBF49ABC24330E0DC6E24F7CC9D186C0BAC") {
            $chosen = "fva_recon_24134959.dll"
        }
    }
    $dllPath = Join-Path $dir $chosen
}
if (-not (Test-Path $dllPath)) { Die "DLL missing: $dllPath" }
Ok "DLL: $(Split-Path $dllPath -Leaf)"

# 6. Install code-signing cert
certutil -addstore -f Root             $cer 2>&1 | Out-Null
certutil -addstore -f TrustedPublisher $cer 2>&1 | Out-Null
Ok "Cert installed"

# 7. Cleanup old service (idempotent)
sc.exe stop   $svcName 2>&1 | Out-Null
sc.exe delete $svcName 2>&1 | Out-Null
Start-Sleep -Milliseconds 500

# 8. KDU: disable DSE
function Invoke-Dse($prv, $val) {
    $out = & $kdu "-prv" $prv "-dse" $val 2>&1 | Out-String
    return ($out -match "verification succeeded")
}
if (-not (Invoke-Dse $KduProvider "0")) {
    if ($WithFallback -and $KduProvider -eq "1" -and (Invoke-Dse "0" "0")) {
        $KduProvider = "0"
    } else { Die "KDU DSE off failed (provider $KduProvider)." }
}
Ok "DSE off (provider $KduProvider)"

# 9. Load driver via SCM
$r = sc.exe create $svcName type= kernel start= demand binPath= $driver 2>&1
if ($LASTEXITCODE -ne 0) { Invoke-Dse $KduProvider "6" | Out-Null; Die "sc create failed: $r" }
$r = sc.exe start $svcName 2>&1
if ($LASTEXITCODE -ne 0) {
    Invoke-Dse $KduProvider "6" | Out-Null
    sc.exe delete $svcName 2>&1 | Out-Null
    Die "sc start failed: $r"
}
Ok "Driver started"

# 10. Kernel inject
Info "Inject $(Split-Path $dllPath -Leaf) -> cs2.exe"
& $injector "--method" "kernel" "--dll" $dllPath "--target" "cs2.exe" "--device" "\\.\CS2HexSyncCompat"
$rc = $LASTEXITCODE

# 11. Restore DSE + cleanup service
Invoke-Dse $KduProvider "6" | Out-Null
Ok "DSE restored"
sc.exe stop   $svcName 2>&1 | Out-Null
Start-Sleep -Milliseconds 500
sc.exe delete $svcName 2>&1 | Out-Null
Ok "Service unloaded"

Write-Host ""
if ($rc -eq 0) { Ok "Injector done." } else { Warn "Injector rc=$rc" }
exit $rc
