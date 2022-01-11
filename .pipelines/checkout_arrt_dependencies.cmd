rem Called by the release pipeline to get dependencies
rem Mostly checks out the internal repository with 3rd party data
rem Also makes sure nuget packages get restored

rem Get Qt, Azure Storage SDK and others through our custom repository
set FILE=%~dp0checkout_arrt_dependencies.ps1
powershell.exe -ExecutionPolicy Unrestricted -NoProfile -WindowStyle Hidden -File "%FILE%" -RepoUrl https://dev.azure.com/msazure/Mixed%%20Reality%%20Services/_git/arr.arrt.dependencies -BranchName main -RemoteName DevDiv -RepoFolder $pwd\..\..\arr.arrt.dependencies
if %errorlevel% neq 0 (
    echo Failed to check out dependency repo %errorlevel%
    exit /B %errorlevel%
)

rem Get ARR and other dependencies from Nuget
%~dp0..\..\arr.arrt.dependencies\Nuget\nuget.exe restore %~dp0..\packages.config -SolutionDirectory %~dp0..
if %errorlevel% neq 0 (
    echo Failed to restore nuget packages %errorlevel%
    exit /B %ERRORLEVEL%
)

exit /B 0