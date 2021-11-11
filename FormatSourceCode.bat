@echo off

echo Formatting...

pushd %~dp0

call :scanRecursive Arrt
call :scanRecursive ArrtModel

echo Done.

popd
exit/b 0

:scanRecursive
pushd %1
for /f "tokens=*" %%G in ('dir /b /s *.cpp *.h *.inl') do (
    rem echo %%G
    clang-format -i %%G
)
popd