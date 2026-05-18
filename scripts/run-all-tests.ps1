param(
    [string]$BuildDir = "build",
    [string]$Config = "Debug",
    [switch]$Build,
    [switch]$Verbose,
    [string]$Regex
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")
$buildPath = Join-Path $repoRoot $BuildDir

if (-not (Test-Path -LiteralPath $buildPath)) {
    Write-Host "Build directory '$buildPath' does not exist. Configure/build first or pass -BuildDir." -ForegroundColor Yellow
    exit 2
}

$ctest = Get-Command ctest -ErrorAction SilentlyContinue
if (-not $ctest) {
    Write-Host "ctest was not found on PATH. Falling back to running test executables found under the build directory." -ForegroundColor Yellow
}

if ($Build) {
    Write-Host "Building project in '$buildPath' ($Config)..." -ForegroundColor Cyan
    & cmake --build "$buildPath" --config $Config --target all --
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
        exit $LASTEXITCODE
    }
}

$testArgs = @(
    "-C", $Config,
    "--test-dir", "$buildPath",
    "--output-on-failure"
)

if ($Verbose) {
    $testArgs += "-V"
}

if ($Regex) {
    $testArgs += @("-R", $Regex)
}

if ($ctest) {
    Write-Host "Running all registered tests via ctest..." -ForegroundColor Cyan
    Write-Host "Command: ctest $($testArgs -join ' ')" -ForegroundColor DarkGray
    & $ctest.Source @testArgs
    exit $LASTEXITCODE
} else {
    # Find test executables under build dir. Match names containing 'test' (case-insensitive) unless a Regex filter is provided.
    Write-Host "Searching for test executables under '$buildPath'..." -ForegroundColor Cyan
    $exeFiles = Get-ChildItem -Path $buildPath -Recurse -Filter *.exe -ErrorAction SilentlyContinue | Where-Object { $_.PSIsContainer -eq $false }
    if ($Regex) {
        $pattern = $Regex
        $exeFiles = $exeFiles | Where-Object { $_.Name -match $pattern }
    } else {
        $exeFiles = $exeFiles | Where-Object { $_.Name -match '(?i)test' }
    }

    if (-not $exeFiles -or $exeFiles.Count -eq 0) {
        Write-Host "No test executables found in '$buildPath'." -ForegroundColor Yellow
        exit 4
    }

    $failures = @()
    foreach ($exe in $exeFiles | Sort-Object FullName) {
        Write-Host "\nRunning: $($exe.FullName)" -ForegroundColor Cyan
        & $exe.FullName
        $code = $LASTEXITCODE
        if ($code -ne 0) {
            Write-Host "FAILED: $($exe.Name) (exit $code)" -ForegroundColor Red
            $failures += @{ Name = $exe.Name; Path = $exe.FullName; ExitCode = $code }
        } else {
            Write-Host "PASSED: $($exe.Name)" -ForegroundColor Green
        }
    }

    if ($failures.Count -gt 0) {
        Write-Host "\nTest run completed: $($failures.Count) failures." -ForegroundColor Red
        foreach ($f in $failures) { Write-Host "- $($f.Name): $($f.Path) (exit $($f.ExitCode))" -ForegroundColor DarkGray }
        exit 100
    } else {
        Write-Host "\nAll test executables passed." -ForegroundColor Green
        exit 0
    }
}
