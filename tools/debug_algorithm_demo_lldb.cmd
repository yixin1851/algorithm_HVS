@echo off
setlocal EnableExtensions

set "ROOT=%~dp0.."
for %%I in ("%ROOT%") do set "ROOT=%%~fI"

set "PROFILE=%~1"
if not defined PROFILE set "PROFILE=%ROOT%\AlgorithmDemo\config\aps_gui_profile.json"

set "EXE=%~2"
if not defined EXE set "EXE=%ROOT%\BUILD\ReleaseAlgorithmDemo\x64\bin\AlgorithmDemod.exe"

set "LLDB=%~3"
if not defined LLDB if defined LLDB_EXE set "LLDB=%LLDB_EXE%"
if not defined LLDB if defined CLION_HOME if exist "%CLION_HOME%\bin\lldb\win\x64\bin\lldb.exe" set "LLDB=%CLION_HOME%\bin\lldb\win\x64\bin\lldb.exe"
if not defined LLDB call :find_lldb "D:\Program Files\JetBrains"
if not defined LLDB call :find_lldb "C:\Program Files\JetBrains"
if not defined LLDB call :find_lldb "%ProgramFiles%\JetBrains"

if not exist "%LLDB%" (
    echo LLDB not found:
    echo   %LLDB%
    echo.
    echo Usage:
    echo   %~nx0 [profile_json] [algorithm_demo_exe] [lldb_exe]
    echo.
    echo Or set one of these environment variables:
    echo   set LLDB_EXE=D:\Program Files\JetBrains\CLion 2026.1\bin\lldb\win\x64\bin\lldb.exe
    echo   set CLION_HOME=D:\Program Files\JetBrains\CLion 2026.1
    exit /b 1
)

if not exist "%EXE%" (
    echo AlgorithmDemo executable not found:
    echo   %EXE%
    echo Build AlgorithmDemo first.
    exit /b 1
)

if not exist "%PROFILE%" (
    echo Profile not found:
    echo   %PROFILE%
    echo Generate it from the PySide6 GUI first.
    exit /b 1
)

echo Launching LLDB for AlgorithmDemo...
echo   exe:     %EXE%
echo   profile: %PROFILE%
echo   lldb:    %LLDB%
echo.
echo Useful LLDB commands:
echo   b file:line
echo        set breakpoint, for example: b ApsRawTestRunner.cpp:1035
echo   n    step over
echo   s    step into
echo   c    continue
echo   bt   backtrace
echo   q    quit
echo.
echo The script will launch AlgorithmDemo and stop at main.cpp.
echo Add more breakpoints in LLDB, then type c to continue.
echo.

"%LLDB%" "%EXE%" ^
  -o "breakpoint set --file main.cpp --line 19" ^
  -o "process launch -- --profile ""%PROFILE%"""

set "LLDB_EXIT=%ERRORLEVEL%"
endlocal & exit /b %LLDB_EXIT%

:find_lldb
if defined LLDB exit /b 0
set "JETBRAINS_DIR=%~1"
if not exist "%JETBRAINS_DIR%" exit /b 0
for /d %%D in ("%JETBRAINS_DIR%\CLion*") do (
    if not defined LLDB if exist "%%~fD\bin\lldb\win\x64\bin\lldb.exe" set "LLDB=%%~fD\bin\lldb\win\x64\bin\lldb.exe"
)
exit /b 0
