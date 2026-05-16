param(
    [string]$BuildDir = "build",
    [switch]$UseCTest
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Push-Location $scriptDir

$buildPath = Resolve-Path -LiteralPath (Join-Path $scriptDir $BuildDir) -ErrorAction SilentlyContinue
if (-not $buildPath) {
    Write-Host "Build directory '$(Join-Path $scriptDir $BuildDir)' does not exist. Please run a CMake build first." -ForegroundColor Yellow
    Pop-Location
    exit 2
}
$buildPath = $buildPath.Path

if ($UseCTest) {
    $ctest = Get-Command ctest -ErrorAction SilentlyContinue
    if ($ctest) {
        & $ctest -C Debug --test-dir $buildPath --output-on-failure
        $code = $LASTEXITCODE
        Pop-Location
        exit $code
    }
}

# Find the smoke_tests executable in the build directory
$exe = Get-ChildItem -Path $buildPath -Recurse -Filter "smoke_tests.exe" -File -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $exe) {
    Write-Host "Could not find smoke_tests.exe in $buildPath" -ForegroundColor Red
    Pop-Location
    exit 3
}

$exePath = $exe.FullName
$outFile = Join-Path $buildPath "..\smoke_output.txt"
Write-Host "Running: $exePath" -ForegroundColor Cyan
Write-Host "Logging to: $outFile" -ForegroundColor Cyan

& "$exePath" > $outFile 2>&1
$code = $LASTEXITCODE

Write-Host "--- Last 200 lines of output ---" -ForegroundColor Gray
Get-Content $outFile -Tail 200 | Write-Host

Pop-Location
exit $code
