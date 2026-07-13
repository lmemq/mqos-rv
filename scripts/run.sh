#!/bin/bash

if ! command -v qemu-system-riscv64 &> /dev/null; then
    echo "qemu-system-riscv64 not found. pls install qemu"
    echo "Macos: brew install qemu"
    echo "Debian: sudo apt install qemu-system-misc"
    read -p "Enter to quit..."
    exit 1
fi

if [[ "$OSTYPE" == "darwin"* ]]; then
    DISPLAY_FLAG="-display cocoa,zoom-to-fit=on"
else
    DISPLAY_FLAG="-display sdl,zoom_to_fit=on"
fi

echo "Starting..."
qemu-system-riscv64 \
    -machine virt \
    -kernel ./kernel \
    -device ramfb \
    -bios none \
    -serial mon:stdio \
    $DISPLAY_FLAG
