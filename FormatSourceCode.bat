@echo off
echo Formatting...
for /r %~dp0 %%f in (*.cpp) do (
    rem echo %%f
    clang-format -i %%f
)
for /r %~dp0 %%f in (*.h) do (
    rem echo %%f
    clang-format -i %%f
)
for /r %~dp0 %%f in (*.inl) do (
    rem echo %%f
    clang-format -i %%f
)
echo Done.
