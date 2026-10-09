$ErrorActionPreference = 'Stop'

$toolDirectory = Join-Path $env:LOCALAPPDATA 'CopyFullPathWithFilename'
$helperPath = Join-Path $toolDirectory 'copy-full-path.ps1'
[System.IO.Directory]::CreateDirectory($toolDirectory) | Out-Null

$helperContent = @'
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Path
)

Set-Clipboard -Value $Path.Replace('\', '/')
'@

$utf8WithoutBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($helperPath, $helperContent, $utf8WithoutBom)

$command = "powershell.exe -NoProfile -NonInteractive -WindowStyle Hidden -ExecutionPolicy Bypass -File `"$helperPath`" `"%1`""
$menuPaths = @(
    'Software\Classes\*\shell\CopyFullPathWithFilename',
    'Software\Classes\Directory\shell\CopyFullPathWithFilename'
)

foreach ($menuPath in $menuPaths) {
    $menuKey = [Microsoft.Win32.Registry]::CurrentUser.CreateSubKey($menuPath)
    $menuKey.SetValue('', '复制完整路径（带文件名）')
    $menuKey.SetValue('Icon', 'shell32.dll,-16763')

    $commandKey = $menuKey.CreateSubKey('command')
    $commandKey.SetValue('', $command)
    $commandKey.Dispose()
    $menuKey.Dispose()
}

Write-Host 'Installed: file and directory context menus now copy full paths with forward slashes.'
