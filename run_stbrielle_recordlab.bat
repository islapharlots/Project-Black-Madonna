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

echo.
echo Compiling St. Brielle Record Laboratory maps...
stbrielle.exe +set fs_game stbrielle +set developer 1 +set con_noPrint 1 +set com_showFPS 0 +set fs_copyfiles 0 +set si_pure 0 +dmap sb_dev_recordlab +dmap sb_dev_recordlab02 +quit
if errorlevel 1 (
    echo.
    echo Record Laboratory map compilation failed.
    echo Check qconsole.log for the compiler error.
    echo.
    exit /b 1
)

echo.
echo Launching St. Brielle Record Laboratory...
stbrielle.exe +set fs_game stbrielle +set developer 1 +set con_noPrint 1 +set com_showFPS 0 +set fs_copyfiles 0 +set si_pure 0 +devmap sb_dev_recordlab

endlocal
