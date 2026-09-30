param(
    [string]$Destination = "",
    [switch]$Force
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($Destination)) {
    $Destination = Split-Path $PSScriptRoot -Parent
}

$Destination = [System.IO.Path]::GetFullPath($Destination)
New-Item -ItemType Directory -Force -Path $Destination | Out-Null

# Pin the dependency bundle so local builds are reproducible.
$DhewmLibsCommit = "57c565984c41356e8b1c4d31f182e763b6ea210a"
$BaseUrl = "https://raw.githubusercontent.com/dhewm/dhewm3-libs/$DhewmLibsCommit/x86_64-w64-mingw32/bin"

# Runtime DLLs imported by the current MSVC St. Brielle executable.
$RuntimeDlls = @(
    "OpenAL32.dll",
    "SDL2.dll",
    "libjpeg-8.dll",
    "libogg-0.dll",
    "libvorbis-0.dll",
    "libvorbisfile-3.dll",
    "zlib1.dll"
)

# PowerShell 5.1 on older Windows configurations may otherwise negotiate an
# obsolete TLS version with GitHub.
try {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
} catch {
    # Newer PowerShell/.NET versions do not need this.
}

Write-Host ""
Write-Host "ST. BRIELLE runtime dependency setup"
Write-Host "Destination: $Destination"
Write-Host ""

$Downloaded = 0
$Existing = 0

foreach ($Dll in $RuntimeDlls) {
    $Output = Join-Path $Destination $Dll

    if ((Test-Path $Output) -and -not $Force) {
        $Item = Get-Item $Output
        if ($Item.Length -gt 0) {
            Write-Host "[OK] $Dll"
            $Existing++
            continue
        }
    }

    $Url = "$BaseUrl/$Dll"
    $Temp = "$Output.download"

    if (Test-Path $Temp) {
        Remove-Item -Force $Temp
    }

    Write-Host "[GET] $Dll"

    $DownloadedOk = $false

    try {
        Invoke-WebRequest -UseBasicParsing -Uri $Url -OutFile $Temp
        $DownloadedOk = $true
    } catch {
        Write-Host "[WARN] Invoke-WebRequest failed for $Dll: $($_.Exception.Message)"
    }

    if (-not $DownloadedOk) {
        if (Test-Path $Temp) {
            Remove-Item -Force $Temp
        }

        $Curl = Get-Command curl.exe -ErrorAction SilentlyContinue
        if ($Curl) {
            Write-Host "[TRY] curl.exe $Dll"
            & $Curl.Source -L --fail --silent --show-error $Url -o $Temp
            if (($LASTEXITCODE -eq 0) -and (Test-Path $Temp)) {
                $DownloadedOk = $true
            }
        }
    }

    if (-not $DownloadedOk) {
        if (Test-Path $Temp) {
            Remove-Item -Force $Temp
        }
        throw "Could not download $Dll from the official dhewm3 dependency bundle."
    }

    $Item = Get-Item $Temp
    if ($Item.Length -le 0) {
        Remove-Item -Force $Temp
        throw "Downloaded file $Dll was empty."
    }

    Move-Item -Force $Temp $Output
    $Downloaded++
}

$Missing = @()
foreach ($Dll in $RuntimeDlls) {
    $Path = Join-Path $Destination $Dll
    if (-not (Test-Path $Path)) {
        $Missing += $Dll
    }
}

if ($Missing.Count -gt 0) {
    throw "Runtime setup incomplete. Missing: $($Missing -join ', ')"
}

Write-Host ""
Write-Host "Runtime dependencies ready. Existing: $Existing  Downloaded: $Downloaded"
Write-Host "Verified: $($RuntimeDlls.Count) DLLs present in $Destination"
Write-Host ""
