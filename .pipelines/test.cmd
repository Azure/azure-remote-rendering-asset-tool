SETLOCAL
SET EXIT_CODE=0

ECHO on
PUSHD C:\source\bin\tests\Debug
dir

FOR /f %%f in ('dir test_*.exe /b') DO (
    CALL "%%f" --gtest_output=xml:output/
    if %errorlevel% neq 0 SET EXIT_CODE=1
)

POPD
ENDLOCAL
exit /B %EXIT_CODE%
