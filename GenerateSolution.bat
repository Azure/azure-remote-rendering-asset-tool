SETLOCAL
SET OUTPUT_DIR=%1
SET CACHEFILE=%OUTPUT_DIR%\CMakeCache.txt

@echo off
IF [%1] EQU [] ( 
    CALL :USAGE
) ELSE IF [%2] EQU [] (
    CALL :GENERATE
) ELSE IF /I "%~2"=="-vs2019" (
    IF EXIST %cachefile% DEL %cachefile%
    CALL :GENERATE -G "Visual Studio 16 2019" -A x64
) ELSE CALL :USAGE
EXIT /b 0

:GENERATE
%CMAKE_PATH%cmake -S %~dp0 -B %OUTPUT_DIR% %*
if %errorlevel% neq 0 (
    echo Failed to generate solution
    exit /b 1
)
EXIT /b 0

:USAGE
ECHO Usage: GenerateSolution.bat Output [-vs2019]
EXIT /b 1