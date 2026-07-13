ifneq ($(shell which riscv64-elf-gcc 2>/dev/null),)
    CC = riscv64-elf-gcc
else ifneq ($(shell which riscv64-unknown-elf-gcc 2>/dev/null),)
    CC = riscv64-unknown-elf-gcc
else ifneq ($(shell which riscv64-unknown-linux-gnu-gcc 2>/dev/null),)
    CC = riscv64-unknown-linux-gnu-gcc
else
    CC = riscv64-linux-gnu-gcc
endif

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
    QEMU_DISPLAY = -display cocoa,zoom-to-fit=on
else
    QEMU_DISPLAY = -display sdl,zoom_to_fit=on
endif

BUILD = build
CFLAGS = -mcmodel=medany -nostdlib -ffreestanding -I include -g

LIBGCC_PATH := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name 2>/dev/null)
ifeq ($(LIBGCC_PATH),)
    LDFLAGS = -T linker.ld -lgcc
else
    LDFLAGS = -T linker.ld $(LIBGCC_PATH)
endif

SRCS = $(wildcard src/*)

KERNEL = $(BUILD)/kernel

all: $(KERNEL)

$(BUILD):
	mkdir -p $(BUILD)

$(KERNEL): $(SRCS) | $(BUILD)
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o $(KERNEL)

run: $(KERNEL)
	qemu-system-riscv64 \
		-machine virt \
		-kernel $(KERNEL) \
		-device ramfb \
		-bios none \
		-serial mon:stdio \
		$(QEMU_DISPLAY)

gdb: $(KERNEL)
	qemu-system-riscv64 \
		-s -S \
		-machine virt \
		-kernel $(KERNEL) \
		-device ramfb \
		-bios none \
		-serial mon:stdio \
		$(QEMU_DISPLAY)

clean:
	rm -rf $(BUILD)/*

.PHONY: all run gdb clean
