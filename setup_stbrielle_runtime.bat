@echo off
setlocal
cd /d "%~dp0"

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\setup_stbrielle_runtime.ps1"
if errorlevel 1 (
    echo.
    echo ST. BRIELLE runtime dependency setup failed.
    echo Check your internet connection and try again.
    echo.
    exit /b 1
)

echo Runtime dependencies are ready.
endlocal
