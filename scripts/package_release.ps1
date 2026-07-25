# 打包 Release：Qt 依赖 + OpenCV DLL → dist/ImageTool
# 用法（在仓库根目录或任意处）：
#   powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$ExeSrc = Join-Path $Root "build\Desktop_Qt_6_7_3_MSVC2019_64bit_Release\release\opencv.exe"
$OutDir = Join-Path $Root "dist\ImageTool"
$WindeployQt = "D:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe"
$OpenCvDll = "D:\opencv\opencv\build\x64\vc16\bin\opencv_world4120.dll"

if (-not (Test-Path $ExeSrc)) {
    Write-Error "找不到 Release 可执行文件:`n  $ExeSrc`n请先在 Qt Creator 中用 Release 完整构建。"
}
if (-not (Test-Path $WindeployQt)) {
    Write-Error "找不到 windeployqt:`n  $WindeployQt"
}
if (-not (Test-Path $OpenCvDll)) {
    Write-Error "找不到 OpenCV Release DLL:`n  $OpenCvDll"
}

Write-Host "==> 输出目录: $OutDir"
if (Test-Path $OutDir) {
    Remove-Item -Recurse -Force $OutDir
}
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

Copy-Item $ExeSrc (Join-Path $OutDir "opencv.exe") -Force
Write-Host "==> 已复制 opencv.exe"

Write-Host "==> 运行 windeployqt ..."
& $WindeployQt --release --compiler-runtime (Join-Path $OutDir "opencv.exe")
if ($LASTEXITCODE -ne 0) {
    Write-Error "windeployqt 失败，退出码 $LASTEXITCODE"
}

Copy-Item $OpenCvDll $OutDir -Force
Write-Host "==> 已复制 opencv_world4120.dll"

# MSVC 运行库（windeployqt 在未设 VCINSTALLDIR 时可能拷不全）
$CrtCandidates = @(
    "D:\C++\production\VC\Redist\MSVC\14.51.36231\x64\Microsoft.VC145.CRT",
    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC"
)
$crtCopied = $false
foreach ($c in $CrtCandidates) {
    if (Test-Path $c) {
        $crtDir = $c
        if ((Split-Path $c -Leaf) -ne "Microsoft.VC145.CRT" -and (Test-Path $c)) {
            $found = Get-ChildItem $c -Directory -Filter "Microsoft.VC*.CRT" -Recurse -ErrorAction SilentlyContinue |
                Where-Object { $_.FullName -match "\\x64\\" } | Select-Object -First 1
            if ($found) { $crtDir = $found.FullName }
        }
        if (Test-Path (Join-Path $crtDir "vcruntime140.dll")) {
            Copy-Item (Join-Path $crtDir "*.dll") $OutDir -Force
            Write-Host "==> 已复制 VC CRT: $crtDir"
            $crtCopied = $true
            break
        }
    }
}
$VcRedist = "D:\Qt\vcredist\vc14.50.35719_VC_redist.x64.exe"
if (Test-Path $VcRedist) {
    Copy-Item $VcRedist (Join-Path $OutDir "VC_redist.x64.exe") -Force
    Write-Host "==> 已附带 VC_redist.x64.exe"
}
if (-not $crtCopied) {
    Write-Warning "未找到本机 VC CRT DLL，目标机若缺运行库请安装 VC_redist.x64.exe"
}

# 可选：附带简要说明
@"
图像处理工具 — 便携包

双击 opencv.exe 即可运行。
主题与英文翻译已打进程序资源，无需额外文件。
首次运行会在本目录下生成 logs/、sessions/。

若提示缺少 VCRUNTIME / MSVCP：
  先运行同目录下的 VC_redist.x64.exe，或确认已拷入 vcruntime140.dll 等。
"@ | Set-Content -Path (Join-Path $OutDir "README.txt") -Encoding UTF8

$ZipPath = Join-Path $Root "dist\ImageTool.zip"
if (Test-Path $ZipPath) { Remove-Item $ZipPath -Force }
Compress-Archive -Path $OutDir -DestinationPath $ZipPath -Force

Write-Host ""
Write-Host "打包完成: $OutDir"
Write-Host "压缩包:   $ZipPath ($([math]::Round((Get-Item $ZipPath).Length/1MB,1)) MB)"
Get-ChildItem $OutDir | Select-Object Name, Length | Format-Table -AutoSize
