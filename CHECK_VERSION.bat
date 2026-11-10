@echo off
REM Shows which game version the code in THIS folder is, so you can tell
REM whether an update was copied into the right place.
setlocal
cd /d "%~dp0"
echo Project folder: %~dp0
echo.
if not exist "%~dp0BMXFreestyleReborn.uproject" (
    echo No BMXFreestyleReborn.uproject in this folder - this is not the project folder.
    goto :done
)
if not exist "%~dp0Source\BMXFreestyleReborn\BMXHUD.cpp" (
    echo Source\BMXFreestyleReborn\BMXHUD.cpp is missing.
    goto :done
)
echo Code version in this folder:
findstr /c:"PARK 01 - v" "%~dp0Source\BMXFreestyleReborn\BMXHUD.cpp"
echo.
if exist "%~dp0BMXFreestyleReborn\Source" (
    echo WARNING: there is a second copy of the project inside this folder:
    echo   %~dp0BMXFreestyleReborn\
    echo The update was probably copied in as a whole folder. Copy that inner
    echo folder's Source and Config up into this folder, replacing the old ones.
)
:done
echo.
pause
