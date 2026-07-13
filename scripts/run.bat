@echo off
chcp 65001 > nul

where qemu-system-riscv64 >nul 2>nul
if %errorlevel% neq 0 (
    echo qemu-system-riscv64 not found
    echo Pls install qemu and add it in PATH
    pause
    exit /b
)

echo Starting...
qemu-system-riscv64.exe ^
    -machine virt ^
    -kernel .\kernel ^
    -device ramfb ^
    -bios none ^
    -serial mon:stdio ^
    -display sdl,zoom_to_fit=on

pause
