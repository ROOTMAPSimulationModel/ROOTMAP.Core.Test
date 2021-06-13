Param(
  [Parameter(Mandatory = $false)]
  [string]$installDir = "C:\vcpkg",
  [Parameter(Mandatory = $false)]
  [switch]$forceCleanInstall
)

if ($forceCleanInstall -eq $true) {
  # Nuke any existing vcpkg installation in $installDir, set up vcpkg again and install all dependencies required by ROOTMAP.
  Invoke-Expression ("$PSScriptRoot\Initialise-Vcpkg.ps1 " + $installDir + " -forceCleanInstall")
}
else {
  # If necessary, set up vcpkg and install all dependencies required by ROOTMAP.
  Invoke-Expression ("$PSScriptRoot\Initialise-Vcpkg.ps1 " + $installDir)
}


# Move to the vcpkg directory
Push-Location $installDir
# Make all vcpkg's installed packages available to the local machine for use in builds
Invoke-Expression ".\vcpkg.exe integrate install" # Per vcpkg's documentation, this command requires admin access when first called.
Pop-Location
