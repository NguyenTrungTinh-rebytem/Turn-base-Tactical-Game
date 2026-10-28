@echo off
setlocal EnableExtensions
cd /d "%~dp0"
if not exist "src\main.cpp" (
 echo ERROR: Place this file in the project root, alongside CMakeLists.txt.
 pause
 exit /b 1
)
set "MSYS=C:\msys64\ucrt64\bin"
if not exist "%MSYS%\g++.exe" (
 echo ERROR: MSYS2 UCRT64 not found at C:\msys64.
 echo Install MSYS2 and in UCRT64 run:
 echo pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-raylib mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
 pause
 exit /b 1
)
set "PATH=%MSYS%;%PATH%"
"%MSYS%\cmake.exe" -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release || goto :failed
"%MSYS%\cmake.exe" --build build-release --parallel 4 || goto :failed
if not exist "build-release\game.exe" goto :failed
if not exist "Game-Windows" mkdir "Game-Windows"
copy /y "build-release\game.exe" "Game-Windows\game.exe" >nul
xcopy /e /i /y "data" "Game-Windows\data" >nul
xcopy /e /i /y "assets" "Game-Windows\assets" >nul
for %%F in (libraylib.dll libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll) do if exist "%MSYS%\%%F" copy /y "%MSYS%\%%F" "Game-Windows\%%F" >nul
rem Copy UCRT runtime DLLs when distributed to other Windows machines.
for %%F in (libssp-0.dll) do if exist "%MSYS%\%%F" copy /y "%MSYS%\%%F" "Game-Windows\%%F" >nul
echo DONE: Game-Windows\game.exe
pause
exit /b 0
:failed
echo BUILD FAILED. Check error messages above.
pause
exit /b 1
