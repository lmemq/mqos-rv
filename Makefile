## Detect Operating System
UNAME_S := $(shell uname -s)

## Automatically detect available RISC-V compiler
ifneq ($(shell which riscv64-elf-gcc 2>/dev/null),)
    CC = riscv64-elf-gcc
else ifneq ($(shell which riscv64-unknown-elf-gcc 2>/dev/null),)
    CC = riscv64-unknown-elf-gcc
else ifneq ($(shell which riscv64-unknown-linux-gnu-gcc 2>/dev/null),)
    CC = riscv64-unknown-linux-gnu-gcc
else
    CC = riscv64-linux-gnu-gcc
endif


## Directories
BUILD = build

## Flags
# -ffreestanding гарантирует компиляцию в окружении без стандартной библиотеки
CFLAGS = -mcmodel=medany -nostdlib -ffreestanding -I include -g

## Resolve libgcc paths dynamically for both macOS and Linux
# This queries the compiler itself to find the internal bare-metal helper library
LIBGCC_PATH := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name 2>/dev/null)
ifeq ($(LIBGCC_PATH),)
    LDFLAGS = -T linker.ld -lgcc
else
    LDFLAGS = -T linker.ld $(LIBGCC_PATH)
endif

## Sources
SRCS = boot.s serial.c qemu_dma.c fb.c kernel.c

## Output
KERNEL = $(BUILD)/kernel

## Default target
all: $(KERNEL)

## Ensure build directory exists
$(BUILD):
	mkdir -p $(BUILD)

## Build kernel
$(KERNEL): $(SRCS) | $(BUILD)
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o $(KERNEL)

## Run in QEMU
run: $(KERNEL)
	qemu-system-riscv64 \
		-machine virt \
		-kernel $(KERNEL) \
		-device ramfb \
		-bios none \
		-serial mon:stdio

## Run with GDB support
gdb: $(KERNEL)
	qemu-system-riscv64 \
		-s -S \
		-machine virt \
		-kernel $(KERNEL) \
		-device ramfb \
		-bios none \
		-serial mon:stdio

## Clean build artifacts
clean:
	rm -rf $(BUILD)/*

.PHONY: all run gdb clean
