@echo off
setlocal
cd /d "%~dp0"

call "%~dp0setup_stbrielle_runtime.bat"
if errorlevel 1 exit /b 1

if not exist "stbrielle.exe" (
    echo.
    echo ST. BRIELLE executable not found.
    echo Build solution\monstergame.sln using the stbrielle project first.
    echo.
    exit /b 1
)

echo Compiling St. Brielle Record Laboratory...
stbrielle.exe +set developer 1 +set fs_copyfiles 0 +set si_pure 0 +dmap sb_dev_recordlab +dmap sb_dev_recordlab02 +devmap sb_dev_recordlab
endlocal
