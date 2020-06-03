set FILE=%~dp0checkout_arrt_dependencies.ps1
powershell.exe -ExecutionPolicy Unrestricted -NoProfile -WindowStyle Hidden -File "%FILE%" -RepoUrl https://dev.azure.com/msazure/Mixed%%20Reality%%20Services/_git/arr.arrt.dependencies -BranchName master -RemoteName DevDiv -RepoFolder $pwd\..\..\arr.arrt.dependencies
if %errorlevel% neq 0 (
    echo Failed to check out dependency repo %errorlevel%
    exit /B %errorlevel%
)

%~dp0..\..\arr.arrt.dependencies\Nuget\nuget.exe restore %~dp0..\packages.config -SolutionDirectory %~dp0..
if %errorlevel% neq 0 (
    echo Failed to restore nuget packages %errorlevel%
    exit /B %ERRORLEVEL%
)

exit /B 0