@echo off

REM This batch file just makes it easier to run GenerateSolution.ps1.
REM Run without arguments to get the default output and Visual Studio version.
REM See the PowerShell script for additional options.

powershell.exe -NoProfile -ExecutionPolicy Bypass "& {& '%~dp0GenerateSolution.ps1' %*}"
