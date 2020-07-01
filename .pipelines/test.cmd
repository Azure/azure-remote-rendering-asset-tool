SETLOCAL EnableDelayedExpansion
SET EXIT_CODE=0

ECHO on
PUSHD C:\source\bin\tests\Debug

FOR /f %%f in ('dir test_*.exe /b') DO (
    CALL "%%f" --gtest_output=xml:output/
    if !ERRORLEVEL! neq 0 SET EXIT_CODE=1
)

POPD

echo exit code = %EXIT_CODE%
exit /B %EXIT_CODE%
