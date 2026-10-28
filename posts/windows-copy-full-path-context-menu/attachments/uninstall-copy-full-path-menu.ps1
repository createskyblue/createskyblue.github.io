$ErrorActionPreference = 'Stop'

$menuPaths = @(
    'Software\Classes\*\shell\CopyFullPathWithFilename',
    'Software\Classes\Directory\shell\CopyFullPathWithFilename'
)

foreach ($menuPath in $menuPaths) {
    [Microsoft.Win32.Registry]::CurrentUser.DeleteSubKeyTree($menuPath, $false)
}

$toolDirectory = Join-Path $env:LOCALAPPDATA 'CopyFullPathWithFilename'
if ([System.IO.Directory]::Exists($toolDirectory)) {
    [System.IO.Directory]::Delete($toolDirectory, $true)
}

Write-Host 'Uninstalled: copy-full-path context menus and helper script were removed.'
