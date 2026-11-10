@echo off
REM Builds the BMXFreestyleReborn editor module and saves all compiler
REM output to BuildLog.txt next to this file. Close Unreal Editor first.
setlocal
cd /d "%~dp0"

REM Find the .uproject here or one folder down (e.g. an extracted zip folder).
set "PROJ="
for %%F in ("%~dp0*.uproject") do set "PROJ=%%~fF"
if not defined PROJ (
    for /d %%D in ("%~dp0*") do (
        for %%F in ("%%~fD\*.uproject") do set "PROJ=%%~fF"
    )
)
if not defined PROJ (
    echo Could not find a .uproject file in "%~dp0" or the folders inside it.
    echo Put this file next to BMXFreestyleReborn.uproject and run it again.
    pause
    exit /b 1
)
echo Project: "%PROJ%"

set "UE_ROOT="
for /f "tokens=2*" %%A in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul') do set "UE_ROOT=%%B"
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"

if not exist "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" (
    echo Could not find Unreal Engine 5.8 at "%UE_ROOT%".
    echo Edit UE_ROOT near the top of this file to your UE_5.8 folder.
    pause
    exit /b 1
)

REM Forward slashes avoid the "G:\\" path mangling seen with Build.bat.
set "PROJ_FWD=%PROJ:\=/%"
set "UBT_DLL=%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll"

REM Use the .NET that ships with Unreal (UE 5.8 needs .NET 10, which most
REM PCs don't have installed). Prefer the win-x64 copy.
set "DOTNET="
for /r "%UE_ROOT%\Engine\Binaries\ThirdParty\DotNet" %%D in (dotnet.exe) do (
    if exist "%%D" (
        echo %%D | findstr /i "win-x64" >nul && set "DOTNET=%%D"
        if not defined DOTNET set "DOTNET=%%D"
    )
)

echo Building with "%UE_ROOT%" ... this can take a few minutes.
if defined DOTNET if exist "%UBT_DLL%" (
    echo Using bundled .NET: "%DOTNET%"
    "%DOTNET%" "%UBT_DLL%" BMXFreestyleRebornEditor Win64 Development -Project="%PROJ_FWD%" -WaitMutex > "%~dp0BuildLog.txt" 2>&1
    goto :result
)
echo Bundled .NET not found - falling back to Build.bat
call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" BMXFreestyleRebornEditor Win64 Development -Project="%PROJ_FWD%" -WaitMutex > "%~dp0BuildLog.txt" 2>&1

:result
if errorlevel 1 (
    echo.
    echo BUILD FAILED. Send BuildLog.txt (in this folder^) to Claude.
    findstr /i /c:"error" "%~dp0BuildLog.txt"
) else (
    echo.
    echo BUILD SUCCEEDED. You can open the .uproject now.
)
pause
