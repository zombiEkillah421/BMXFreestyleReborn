@echo off
REM Builds the BMXFreestyleReborn editor module and saves all compiler
REM output to BuildLog.txt next to this file. Close Unreal Editor first.
setlocal
cd /d "%~dp0"

set "UE_ROOT="
for /f "tokens=2*" %%A in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul') do set "UE_ROOT=%%B"
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"

if not exist "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" (
    echo Could not find Unreal Engine 5.8 at "%UE_ROOT%".
    echo Edit UE_ROOT near the top of this file to your UE_5.8 folder.
    pause
    exit /b 1
)

echo Building with "%UE_ROOT%" ... this can take a few minutes.
call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" BMXFreestyleRebornEditor Win64 Development -Project="%~dp0BMXFreestyleReborn.uproject" -WaitMutex > BuildLog.txt 2>&1

if errorlevel 1 (
    echo.
    echo BUILD FAILED. Send BuildLog.txt (in this folder^) to Claude.
    findstr /i /c:"error" BuildLog.txt
) else (
    echo.
    echo BUILD SUCCEEDED. You can open BMXFreestyleReborn.uproject now.
)
pause
