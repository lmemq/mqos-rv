# mqos (RISC-V 64 OS)

A lightweight, bare-metal operating system for the **64-bit RISC-V** architecture. 

The project was started from Bare metal Qemu-Ramfb implementation for RISC-V, but came into the simple hobby OS.

---

## 📂 Project Structure

* `src/` — Sources
* `include/` — Header files
* `scripts/` — Scripts for fast-testing OS on windows, gnu/linux and macos
* `linker.ld` — The linker script
* `Makefile` — File for building mqos on gnu/linux and macos
* `LICENSE` — Project license

---

## TODO / Features

### 🟢 Completed
- [x] **RISC-V 64-bit Architecture** — forked ramfb driver, kernel works.
- [x] **Cross-Platform Makefile** — The build system auto-detects cross-compilers on both macOS and Linux.
- [x] **Heap Allocator** — ram partitions, kmalloc(), kfree() and getting busy memory.
- [x] **Double Buffering Screen** — `back_buffer` in the heap and flash().
- [x] **Hide fb struct from kernel**

### 🟡 In Progress
- [ ] **Bitmap Font**
- [ ] **On-Screen Text Console**
- [ ] **Custom `kprintf()`** — Implement a format parser (`%d`, `%x`, `%s`) to print debug logs to both UART and the screen.

### 🔴 Future
- [ ] **RISC-V Trap Handler** — Write assembly code (`mtvec`) to catch CPU exceptions and prevent unrecoverable kernel panics.
- [ ] **Hardware Timer** — Integrate the RISC-V hardware timer (`mtime`/`mtimecmp`) for accurate time ticks and a `ksleep()` function.
- [ ] **Keyboard Input** — Read keypresses from QEMU to allow user control.
- [ ] **Drivers** - drivers for many hardware. 
- [ ] **QOI Support**
- [ ] **Rings** - third ring mode for ram and userspace.

---

## How to Run Pre-Built Binaries

U dont need to build mqos as we have built it for you

1. Download latest **mqos.zip** from [releases page](https://github.com/lmemq/mqos-rv/releases)
2. Extract the ZIP archive.
3. Make sure you have QEMU (`qemu-system-riscv64`) installed.
4. Launch the OS:
   * **macOS / Linux**: `run.sh`
   * **Windows**: `run.bat`

*Tip: U can simply expand qemu window for best experiense!*

---

## How to Build

### 1. Prerequisites
You need a RISC-V cross-compiler and the QEMU system emulator:
* **macOS**: `brew install riscv64-elf-gcc qemu`
* **Debian**: `sudo apt install gcc-riscv64-unknown-elf qemu-system-misc`
* **Other**: `U can just find instructions in the Net`

### 2. Build Commands
* **Compile Kernel**: `make all` (The compiled ELF image will appear in `build/kernel`)
* **Compile and run**: `make run`
* **Debug with GDB**: `make gdb`
* **Clean**: `make clean`

---

## About mqos inside

* RAM is: [kernel] -> [stack, 64 KB] -> [heap, 64 MB] -> [ramfb (all pixels on qemu screen are connected to Ram), ±3.51 MB]

## The Lore Behind the Name

* **The Child Dream:** Years ago, before knowing how to compile a single line of code, lmemq dreamed of building a custom "Windows-like" system ("mos")
* **Nickname:**
  * **`lme`** made from `lime` gaming nickname
  * **`mq`** visual pseudo-signature, made from MineCraft, mc, mq
* **Final name:** Merging `mq` with the `mos` causes `mqos`

---

## 👥 Credits & Acknowledgments

* **@luickk** & **[@CityAceE](https://github.com/CityAceE)** — Huge thanks for the original `qemu-ramfb` driver. Their work is a graphic base of mqos-rv.
* **[@lmemq](https://github.com/lmemq)** — Author of RISC-V mqos, mqos-rv.
