# Run the copy of GitVersion installed in the packages directory to get GitVersion information for this project.
$gitVersionExe = Get-ChildItem -Recurse ..\.. GitVersion.exe
$gitVersionJson = & $gitVersionExe /output json
Write-Host "GitVersion output:"
Write-Host $gitVersionJson
$gitVersionProperties = $gitVersionJson | Out-String | ConvertFrom-Json
$ver = $gitVersionProperties.NuGetVersionV2

# Generate the content of `RmVersion.cpp`.
$newVersionCpp = @"
#include <string>

extern const std::string rootmapCoreVersion = "$ver";
"@

# Determine if `RmVersion.cpp` already exists, and diff it against the new if it does.
$versionCppExists = Test-Path .\common\RmVersion.cpp
$writeNewVersionCpp = $true

if ($versionCppExists) {
  $previousVersionCpp = Get-Content -Raw .\common\RmVersion.cpp
  $writeNewVersionCpp = $previousVersionCpp.Trim() -ne $newVersionCpp.Trim()
}

# Rewrite `RmVersion.cpp` only if it has changed, to avoid needless recompilations.
if ($writeNewVersionCpp) {
  Write-Host "Setting version in code to $ver"
  $newVersionCpp | Out-File -FilePath .\common\RmVersion.cpp
}
else {
  Write-Host "Version in code already set to $ver"
}
