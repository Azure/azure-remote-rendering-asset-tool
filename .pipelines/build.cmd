SETLOCAL
SET ARRT_SRC=C:\source
SET DEPS_DIR=C:\arr.arrt.dependencies
SET Qt5_DIR=%DEPS_DIR%\Qt\5.13.1\msvc2017_64

REM Set developer environment for VS 2017
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64 -winsdk=10.0.16299.0
echo on
cd C:\source

SETLOCAL
SET CACHEFILE=CMakeCache.txt
SET NUGET_RESTORE=false

cmake -G "Visual Studio 15 2017" -A x64
if %errorlevel% neq 0 (
    echo Failed to generate solution %errorlevel%
    exit /B 1
)

dir

msbuild /m /p:Configuration=Debug /p:Platform=x64 Arrt.sln

if %errorlevel% neq 0 (
    echo Failed to build solution properly. %errorlevel%
    exit /B 1
)

msbuild /m /p:Configuration=Release /p:Platform=x64 Arrt.sln

if %errorlevel% neq 0 (
    echo Failed to build solution properly. %errorlevel%
    exit /B 1
)

ENDLOCAL