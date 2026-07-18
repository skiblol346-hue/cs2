@echo off
setlocal
cd /d "%~dp0"

REM Elevate self if not admin.
net session >nul 2>&1
if errorlevel 1 (
    echo [*] Requesting admin elevation...
    powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b
)

REM Bypass execution policy for this invocation only.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Launch-Inject.ps1" %*
set RC=%ERRORLEVEL%

echo.
echo [Launcher] Exit code: %RC%
pause
exit /b %RC%
