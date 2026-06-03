param(
    [string]$BuildDir = "build",
    [string]$Config = "Release",  # Defaulting to Release since that matches your CI targets
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

# ============================================================================
# --- CRITICAL CROSS-PLATFORM SYSTEM PATH INJECTION ---
# ============================================================================
# This automatically reads your local project cache to find where Qt and OpenSSL live 
# and safely hooks them into the local shell profile environment dynamically.
if ($IsWindows -or $env:OS -like "*Windows*") {
    $cmakeCacheFile = Join-Path $buildPath "CMakeCache.txt"
    if (Test-Path $cmakeCacheFile) {
        Write-Host "Analyzing CMakeCache.txt to auto-resolve dependency paths..." -ForegroundColor DarkGray
        
        # 1. Extract OpenSSL path from cache
        $openSslLine = Get-Content $cmakeCacheFile | Where-Object { $_ -match "^OPENSSL_ROOT_DIR:PATH=" }
        if ($openSslLine) {
            $openSslRoot = ($openSslLine -split "=")[1].Replace('/', '\')
            $openSslBin = Join-Path $openSslRoot "bin"
            if (Test-Path $openSslBin) {
                $env:PATH = "$openSslBin;" + $env:PATH
                Write-Host "-> Locally injected OpenSSL binaries path: $openSslBin" -ForegroundColor DarkGray
            }
        }

        # 2. Extract Qt folder from cache
        $qtLine = Get-Content $cmakeCacheFile | Where-Object { $_ -match "^Qt6_DIR:PATH=" }
        if ($qtLine) {
            $qtDir = ($qtLine -split "=")[1].Replace('/', '\')
            # Climb out of lib/cmake/Qt6 up to the bin folder
            $qtBin = Join-Path $qtDir "..\..\..\bin"
            if (Test-Path $qtBin) {
                $env:PATH = "$qtBin;" + $env:PATH
                Write-Host "-> Locally injected Qt binaries path: $qtBin" -ForegroundColor DarkGray
            }
        }
    }
}
# ============================================================================

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
        Write-Host "`nRunning: $($exe.FullName)" -ForegroundColor Cyan
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
        Write-Host "`nTest run completed: $($failures.Count) failures." -ForegroundColor Red
        foreach ($f in $failures) { Write-Host "- $($f.Name): $($f.Path) (exit $($f.ExitCode))" -ForegroundColor DarkGray }
        exit 100
    } else {
        Write-Host "`nAll test executables passed." -ForegroundColor Green
        exit 0
    }
}