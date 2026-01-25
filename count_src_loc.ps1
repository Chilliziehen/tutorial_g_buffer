param(
    [string]$Root = (Join-Path $PSScriptRoot "src")
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $Root)) {
    Write-Error "Path not found: $Root"
    exit 1
}

# Collect .cpp/.h under src (recursive)
$files = Get-ChildItem -Path $Root -Recurse -File -Include *.cpp,*.h

if ($files.Count -eq 0) {
    Write-Host "No .cpp/.h files found under: $Root"
    exit 0
}

$rows = foreach ($f in $files) {
    $lineCount = (Get-Content -LiteralPath $f.FullName | Measure-Object -Line).Lines
    [pscustomobject]@{
        File = $f.FullName
        Lines = [int]$lineCount
        Extension = $f.Extension
    }
}

$total = ($rows | Measure-Object -Property Lines -Sum).Sum
$totalCpp = ($rows | Where-Object { $_.Extension -ieq ".cpp" } | Measure-Object -Property Lines -Sum).Sum
$totalH = ($rows | Where-Object { $_.Extension -ieq ".h" } | Measure-Object -Property Lines -Sum).Sum

Write-Host "Root: $Root"
Write-Host ("Files: {0}  (.cpp: {1}, .h: {2})" -f $files.Count, ($files | Where-Object Extension -ieq ".cpp").Count, ($files | Where-Object Extension -ieq ".h").Count)
Write-Host ("Total lines: {0}  (cpp: {1}, h: {2})" -f $total, $totalCpp, $totalH)
Write-Host ""

# Per-file breakdown (largest first)
$rows | Sort-Object Lines -Descending | Format-Table -AutoSize

