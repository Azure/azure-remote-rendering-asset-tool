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
& cmake.exe --version

if ($null -eq (Get-Command "nuget.exe" -ErrorAction SilentlyContinue)) 
{ 
	throw "Unable to find nuget.exe in your PATH environment variable."
}

if ($null -eq (Get-Command "cmake.exe" -ErrorAction SilentlyContinue)) 
{ 
	throw "Unable to find cmake.exe in your PATH environment variable."
}

if ($null -eq $env:Qt6_DIR)
{ 
	Write-Host "Environment variable 'Qt6_DIR' is not set."
	#throw "Environment variable 'Qt6_DIR' is not set."
}
else 
{
	Write-Host "Environment variable Qt6_DIR is set to: '$env:Qt6_DIR'."
}

if (-not ($null -eq $env:Qt6_DIR) -and (-not (Test-Path $env:Qt6_DIR)))
{ 
	#throw "Environment variable Qt6_DIR points to non-existing directory: '$env:Qt6_DIR'."
}

$CacheFile = $Destination + "\CMakeCache.txt"
$VcpkgPath = $Destination + "\vcpkg"

if (Test-Path -Path $CacheFile -PathType leaf) {
	Remove-Item $CacheFile
}

Write-Host ""
Write-Host "=== Running vcpkg ==="
Write-Host ""

if ((Test-Path -Path $VcpkgPath) -eq $false) {
	git clone https://github.com/microsoft/vcpkg "$Destination\vcpkg"
	
	# could check out a fixed commit, but we'll just try latest for now
}

# bootstrap vcpkg
&$VcpkgPath\bootstrap-vcpkg.bat

# get Azure Storage SDK through vcgpk
&$VcpkgPath\vcpkg.exe install azure-storage-blobs-cpp:x64-windows 

if (!$?) {
	throw "Vcpkg error $LASTEXITCODE, see log above."
}

# get CppRest SDK through vcgpk
&$VcpkgPath\vcpkg.exe install cpprestsdk:x64-windows

if (!$?) {
	throw "Vcpkg error $LASTEXITCODE, see log above."
}

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

& cmake.exe $CMAKE_ARGS

if (!$?) {
	throw "CMake error $LASTEXITCODE, see log above."
}
