SETLOCAL
SET OUTPUT_DIR=%1
SET CACHEFILE=%OUTPUT_DIR%\CMakeCache.txt

@echo off
IF [%1]==[] (
    CALL :USAGE
) ELSE IF [%2]==[] (
    CALL :GENERATE
) ELSE IF /I %~2 == -vs2017 (
    IF EXIST %CACHEFILE% DEL %CACHEFILE%
    CALL :GENERATE -G "Visual Studio 15 2017" -A x64
) ELSE IF /I %~2 == -vs2019 (
    IF EXIST %cachefile% DEL %cachefile%
    CALL :GENERATE -G "Visual Studio 16 2019" -A x64
) ELSE (
    CALL :USAGE
)
EXIT /b 0

:GENERATE
%CMAKE_PATH%cmake -S %~dp0 -B %OUTPUT_DIR% %*
EXIT /b 0

:USAGE
ECHO Usage: GenerateSolution.bat Output [-vs2017] [-vs2019]
EXIT /b 0