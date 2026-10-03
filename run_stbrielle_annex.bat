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

set "SBROOT=%CD%"
set "SBSAVE=%APPDATA%\StBrielle\stbrielle"

if exist "%SBROOT%\stbrielle\maps\sb00_annex.proc" del /q "%SBROOT%\stbrielle\maps\sb00_annex.proc"
if exist "%SBROOT%\stbrielle\maps\sb00_annex.cm" del /q "%SBROOT%\stbrielle\maps\sb00_annex.cm"

echo.
echo Compiling Municipal Annex...
echo Base path: %SBROOT%
echo Dev path:  %SBROOT%
echo.

stbrielle.exe +set fs_basepath "%SBROOT%" +set fs_devpath "%SBROOT%" +set fs_game stbrielle +set developer 1 +set con_noPrint 1 +set com_showFPS 0 +set fs_copyfiles 0 +set si_pure 0 +bind e _frob +dmap2 sb00_annex +quit

if exist "%SBSAVE%\qconsole.log" copy /y "%SBSAVE%\qconsole.log" "%SBROOT%\annex_compile.log" >nul

if not exist "%SBROOT%\stbrielle\maps\sb00_annex.proc" (
    echo.
    echo ERROR: sb00_annex.proc was not created.
    echo See annex_compile.log for the dmap output.
    echo.
    pause
    exit /b 1
)

if not exist "%SBROOT%\stbrielle\maps\sb00_annex.cm" (
    echo.
    echo ERROR: sb00_annex.cm was not created.
    echo See annex_compile.log for the dmap output.
    echo.
    pause
    exit /b 1
)

echo.
echo Compile verified:
echo   stbrielle\maps\sb00_annex.proc
echo   stbrielle\maps\sb00_annex.cm
echo.
echo Launching Municipal Annex...
echo Interaction: E
echo.

stbrielle.exe +set fs_basepath "%SBROOT%" +set fs_devpath "%SBROOT%" +set fs_game stbrielle +set developer 1 +set con_noPrint 1 +set com_showFPS 0 +set fs_copyfiles 0 +set si_pure 0 +bind e _frob +devmap sb00_annex

if exist "%SBSAVE%\qconsole.log" copy /y "%SBSAVE%\qconsole.log" "%SBROOT%\annex_runtime.log" >nul

endlocal
