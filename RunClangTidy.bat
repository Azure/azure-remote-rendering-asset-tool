@echo off
CALL %~dp0BuildWithNinja.bat %1

set checks="clang-diagnostic-*,clang-analyzer-*,-modernize-*,performance-*,-cppcoreguidelines-*"

echo Analyzing...
for /r %~dp0 %%f in (*.cpp) do (
    echo %%f
    "%~f2"clang-tidy --checks=%checks% --warnings-as-errors=%checks% -p %1 %%f
)
echo Done.
