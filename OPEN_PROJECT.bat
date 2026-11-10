@echo off
REM Opens the project directly in Unreal Editor 5.8 (skips the version
REM selector). Run BUILD_EDITOR.bat first after pulling code changes.
setlocal
cd /d "%~dp0"
set "UE_ROOT="
for /f "tokens=2*" %%A in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul') do set "UE_ROOT=%%B"
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
if not exist "%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" (
    echo Could not find UnrealEditor.exe under "%UE_ROOT%".
    pause
    exit /b 1
)
start "" "%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0BMXFreestyleReborn.uproject"
