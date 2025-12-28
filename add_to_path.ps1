# 需要管理员权限的PowerShell脚本
# 用于将MSYS2路径永久添加到系统PATH环境变量

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " 添加MSYS2到系统PATH" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 检查管理员权限
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "[错误] 需要管理员权限运行此脚本" -ForegroundColor Red
    Write-Host ""
    Write-Host "请右键点击此文件，选择'以管理员身份运行'" -ForegroundColor Yellow
    Write-Host ""
    pause
    exit 1
}

Write-Host "[OK] 已获取管理员权限" -ForegroundColor Green
Write-Host ""

# 定义要添加的路径
$msys2Path = "D:\software\msys64"
$pathsToAdd = @(
    "$msys2Path\usr\bin",
    "$msys2Path\mingw64\bin"
)

# 获取当前系统PATH
$currentPath = [Environment]::GetEnvironmentVariable("Path", "Machine")
$pathArray = $currentPath -split ';'

Write-Host "当前系统PATH包含 " + $pathArray.Length + " 个路径" -ForegroundColor Cyan
Write-Host ""

# 检查并添加路径
$added = 0
foreach ($newPath in $pathsToAdd) {
    $pathExists = $pathArray -contains $newPath

    if (-not $pathExists) {
        Write-Host "添加路径: $newPath" -ForegroundColor Yellow
        $pathArray += $newPath
        $added++
    } else {
        Write-Host "路径已存在: $newPath" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan

if ($added -gt 0) {
    Write-Host "将更新系统PATH环境变量..." -ForegroundColor Yellow
    Write-Host ""

    $newPathString = $pathArray -join ';'

    try {
        [Environment]::SetEnvironmentVariable("Path", $newPathString, "Machine")
        Write-Host "[成功] 已添加 $added 个路径到系统PATH" -ForegroundColor Green
        Write-Host ""
        Write-Host "请重新启动命令提示符或PowerShell以使更改生效" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "或者运行 setup_env.bat 临时设置环境变量" -ForegroundColor Cyan
    } catch {
        Write-Host "[错误] 无法更新PATH: $_" -ForegroundColor Red
    }
} else {
    Write-Host "所有路径都已存在，无需添加" -ForegroundColor Green
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "验证安装:" -ForegroundColor Cyan
Write-Host ""

# 验证工具是否可用
$tools = @("flex", "bison", "g++", "make")
foreach ($tool in $tools) {
    $result = Get-Command $tool -ErrorAction SilentlyContinue
    if ($result) {
        Write-Host "[$tool] ✓ 已安装" -ForegroundColor Green
    } else {
        Write-Host "[$tool] ✗ 未找到" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

pause
