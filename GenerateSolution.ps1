param 
(
	[Parameter(Mandatory = $False)] [string] $Destination = "$PSScriptRoot\Workspace",
	[Parameter(Mandatory = $False)] [ValidateSet('vs2019', 'vs2022')] $Solution = "vs2019"
)

Write-Host ""
Write-Host "=== Generating ARRT Solution ==="
Write-Host ""
Write-Host "Destination Path: $Destination"
Write-Host "Solution for: $Solution"

if ($null -eq (Get-Command "nuget.exe" -ErrorAction SilentlyContinue)) 
{ 
	throw "Unable to find nuget.exe in your PATH environment variable."
}

if ($null -eq (Get-Command "cmake.exe" -ErrorAction SilentlyContinue)) 
{ 
	throw "Unable to find cmake.exe in your PATH environment variable."
}

$CacheFile = $Destination + "\CMakeCache.txt"
$VcpkgPath = $Destination + "\vcpkg"

if (Test-Path -Path $CacheFile -PathType leaf) {
	Remove-Item $CacheFile
}

Write-Host ""
Write-Host "=== Preparing vcpkg ==="
Write-Host ""

if ((Test-Path -Path $VcpkgPath) -eq $false) {
	git clone https://github.com/microsoft/vcpkg "$Destination\vcpkg"
	
	# could check out a fixed commit, but we'll just try latest for now
}

&$VcpkgPath\bootstrap-vcpkg.bat
#$Env:Path = "$PSScriptRoot\Tools\CMake\bin;" + $Env:Path
#$Env:NUGET_PATH = "$PSScriptRoot\Tools\Nuget\"
&$VcpkgPath\vcpkg.exe install azure-storage-blobs-cpp:x64-windows 

Write-Host ""
Write-Host "=== Running CMake ==="
Write-Host ""

$CMAKE_ARGS = @("-S", "$PSScriptRoot")
$CMAKE_ARGS += @("-B", "$Destination")
$CMAKE_ARGS += @("-A", "x64")
$CMAKE_ARGS += "-G"

if ($Solution -eq "vs2022") {
	$CMAKE_ARGS += "Visual Studio 17 2022"
} else {
	$CMAKE_ARGS += "Visual Studio 16 2019"
}

$CMAKE_ARGS += "-DCMAKE_TOOLCHAIN_FILE=$VcpkgPath\scripts\buildsystems\vcpkg.cmake"
$CMAKE_ARGS += "-DUSE_NEW_AZURE_STORAGE_SDK:BOOL=ON"

#& $PSScriptRoot\Tools\CMake\bin\cmake.exe $CMAKE_ARGS
& cmake.exe $CMAKE_ARGS

if (!$?) {
	throw "CMake Error $LASTEXITCODE, see log above."
}
