$ErrorActionPreference = 'Stop'

$helperPath = Join-Path $env:LOCALAPPDATA 'CopyFullPathWithFilename\copy-full-path.ps1'
$registryPaths = @(
    'Software\Classes\*\shell\CopyFullPathWithFilename\command',
    'Software\Classes\Directory\shell\CopyFullPathWithFilename\command'
)

if (-not (Test-Path -LiteralPath $helperPath)) {
    throw "Helper script is missing: $helperPath"
}

foreach ($registryPath in $registryPaths) {
    $commandKey = [Microsoft.Win32.Registry]::CurrentUser.OpenSubKey($registryPath)
    if ($null -eq $commandKey) {
        throw "Context-menu command is missing: HKCU\$registryPath"
    }

    $registeredCommand = [string]$commandKey.GetValue('')
    $commandKey.Dispose()

    if (-not $registeredCommand.Contains('-File') -or -not $registeredCommand.Contains($helperPath)) {
        throw "Context-menu command does not call the helper through -File: $registeredCommand"
    }
}

$originalClipboard = Get-Clipboard -Raw -ErrorAction SilentlyContinue
$samplePath = 'C:\测试目录\path with spaces & symbols.txt'
$expectedPath = $samplePath.Replace('\', '/')

try {
    Set-Clipboard -Value '__copy_full_path_test_pending__'
    $process = Start-Process -FilePath 'powershell.exe' -ArgumentList @(
        '-NoProfile',
        '-NonInteractive',
        '-WindowStyle', 'Hidden',
        '-ExecutionPolicy', 'Bypass',
        '-File', "`"$helperPath`"",
        "`"$samplePath`""
    ) -Wait -PassThru -WindowStyle Hidden

    if ($process.ExitCode -ne 0) {
        throw "Helper script exited with code $($process.ExitCode)."
    }

    $actualPath = Get-Clipboard -Raw
    if ($actualPath -ne $expectedPath) {
        throw "Clipboard mismatch. Expected: $expectedPath; actual: $actualPath"
    }

    Write-Host 'PASS: file and directory menus are registered; clipboard uses forward slashes.'
}
finally {
    if ($null -ne $originalClipboard) {
        Set-Clipboard -Value $originalClipboard
    }
}
