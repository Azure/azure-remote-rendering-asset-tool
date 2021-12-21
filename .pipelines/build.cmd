SETLOCAL
SET ARRT_SRC=C:\source
SET DEPS_DIR=C:\arr.arrt.dependencies
SET Qt5_DIR=%DEPS_DIR%\Qt\5.13.1\msvc2017_64

REM Set developer environment for VS 2019
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64 -winsdk=10.0.16299.0
echo on
pushd C:\source

SET CACHEFILE=CMakeCache.txt
SET NUGET_RESTORE=false
REM SET ARRT_VERSION=v%CDP_PACKAGE_VERSION_SEMANTIC%

cmake --version
cmake -G "Visual Studio 16 2019" -A x64
if %errorlevel% neq 0 (
    echo Failed to generate solution %errorlevel%
    goto errorExit
)

msbuild /m /p:Configuration=Debug /p:Platform=x64 Arrt.sln

if %errorlevel% neq 0 (
    echo Failed to build debug solution. %errorlevel%
    goto errorExit
)

msbuild /m /p:Configuration=Release /p:Platform=x64 Arrt.sln

if %errorlevel% neq 0 (
    echo Failed to build release solution. %errorlevel%
    goto errorExit
)

popd
exit /B 0

:errorExit
popd
exit /B 1

