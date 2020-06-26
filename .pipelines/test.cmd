SETLOCAL
SET EXIT_CODE=0

ECHO on
PUSHD C:\source\bin\tests\Debug
dir

where QT5GUID.DLL
where QT5CORED.DLL
where WASTORAGED.DLL
where CPPREST140D_2_9.DLL
where REMOTERENDERINGCLIENT.DLL
where D3D11.DLL
where CRYPT32.DLL
where KERNEL32.DLL
where OLE32.DLL
where MSVCP140D.DLL
where CONCRT140D.DLL
where VCRUNTIME140D.DLL
where UCRTBASED.DLL

FOR /f %%f in ('dir test_*.exe /b') DO (
    CALL "%%f" --gtest_output=xml:output/
    if %errorlevel% neq 0 SET EXIT_CODE=1
)

POPD
ENDLOCAL
exit /B %EXIT_CODE%
