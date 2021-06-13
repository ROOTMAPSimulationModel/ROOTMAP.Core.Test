Param(
  [Parameter(Mandatory = $true)]
  [string]$installDir,
  [Parameter(Mandatory = $false)]
  [switch]$forceCleanInstall
)

$forceCleanInstall = $forceCleanInstall -eq $true
$installRequired = $true
$dirExists = Test-Path $installDir

# If vcpkg directory exists and a forced clean installation is not requested,
# check to ensure vcpkg is actually installed correctly.
if ($dirExists -and -not $forceCleanInstall) {
  Push-Location $installDir
  if (Test-Path ".\vcpkg.exe") {
    # If vcpkg exe exists, ensure it reports a valid version
    $ver = Invoke-Expression ".\vcpkg.exe version"
    if (($ver -Match "Vcpkg package management program version").Length -gt 0) {
      Write-Host "Vcpkg detected in ${installDir}: $ver"
      # Ensure the required dependencies are installed.
      [Array]$deps = @(Invoke-Expression ".\vcpkg.exe list")
      Write-Host "Vcpkg libraries installed:"
      $deps | Foreach-Object { Write-Host $_ }
      $hasWxWidgets = ($deps -Match "wxwidgets:x86-windows").Length -gt 0
      $hasTinyXml = ($deps -Match "tinyxml:x86-windows").Length -gt 0
      if ($hasWxWidgets -and $hasTinyXml) {
        Write-Host "All vcpkg dependencies required for ROOTMAP have been detected.`n"
        # If all these checks pass, consider the vcpkg installation to be good and don't reinstall it.
        $installRequired = $false
      }
    }
  }
  Pop-Location
}

# Only delete the directory if forced.
if ($dirExists -and $forceCleanInstall) {
  Remove-Item $installDir -Force -Recurse
  $dirExists = $false
}

if ($installRequired -and -not $dirExists) {
  Write-Host ("Cloning vcpkg to " + $installDir + "...")

  # Largely borrowed from https://github.com/Microsoft/azure-pipelines-image-generation/commit/5230c9f52f55e99bb5fac831b8c78c1d5e081660
  $uri = 'https://github.com/microsoft/vcpkg.git'

  # Check out the latest revision of vcpkg
  git.exe clone --depth=1 $uri $installDir -q

  Push-Location $installDir

  # Initialise vcpkg and install dependencies required by ROOTMAP.
  Write-Host "Bootstrapping vcpkg..."
  Invoke-Expression ".\bootstrap-vcpkg.bat"
  Write-Host "Updating vcpkg portfiles..."
  Invoke-Expression ".\vcpkg.exe update"
  Write-Host "Installing ROOTMAP dependencies..."
  Invoke-Expression ".\vcpkg.exe install tinyxml:x86-windows wxwidgets:x86-windows"
  Write-Host ("Finished.")
  Pop-Location
}
elseif ($installRequired) {
  throw "Directory exists but does not contain a functional and suitably-configured installation of vcpkg.`nIf the target directory is correct, pass -forceCleanInstall to install a clean copy of vcpkg therein.`nWARNING: this will delete all data in the directory!`n"
}
else {
  Write-Host "Vcpkg and dependencies required for ROOTMAP are already installed.`nTo do a clean reinstall, pass -forceCleanInstall to this script.`n"
}
