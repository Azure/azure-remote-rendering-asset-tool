SETLOCAL
SET ARRT_SRC=C:\source
SET DEPS_DIR=C:\arr.arrt.dependencies
SET Qt5_DIR=%DEPS_DIR%\Qt\5.13.1\msvc2017_64

SET "VS_LOCATION=C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7"
SET EXIT_CODE=0

REM Set developer environment for VS 2017
CALL "%VS_LOCATION%\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64 -winsdk=10.0.16299.0
ECHO on
PUSHD C:\source\bin\tests\Debug

FOR /f %%f in ('dir test_*.exe /b') DO (
    CALL VSTest.Console.exe "%%f" /logger:trx /Platform:x64 /TestAdapterPath:"%VS_LOCATION%\Common7\IDE\Extensions"
    if %errorlevel% neq 0 SET EXIT_CODE=1
)

POPD
ENDLOCAL
exit /B %EXIT_CODE%
