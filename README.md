# mqos (RISC-V 64 OS)

A lightweight, bare-metal operating system for the **64-bit RISC-V** architecture. 

The project was started from fork of bare metal Qemu-Ramfb implementation for RISC-V, but came into the simple hobby OS.

<video src="https://github.com/user-attachments/assets/63913c8c-7369-478f-afe5-a9bc164c0cbd" controls autoplay loop muted width="100%"></video>

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
- [x] **Double Buffering Screen** — `back_buffer` in the heap and flush().
- [x] **Hide fb struct from kernel**
- [x] **Threading with yield()** - cooperative mutithreading
- [x] **RISC-V Trap Handler** — can catch cpu exceptions and other traps
- [x] **Hardware Timer** — timer for ksleep() and preemptive multithreading
- [x] **QOI Image Support**
- [x] **On-Screen Text Console**
- [x] **Custom `kprintf()`** — Implement a format parser (`%d`, `%x`, `%s`) to print

### 🟡 In Progress / Near Future
- [ ] **Keyboard Input** — Read keypresses from QEMU to allow user control.

### 🔴 Future
- [ ] **Drivers** - drivers for many hardware. 
- [ ] **Virtual Memory** — virtual memory management for userspace.
- [ ] **Rings** - third ring mode for ram and userspace.

---

## How to Run Pre-Built Binaries

You dont need to build mqos as we have built it for you

1. Download latest **mqos.zip** from [releases page](https://github.com/lmemq/mqos-rv/releases)
2. Extract the ZIP archive.
3. Make sure you have QEMU (`qemu-system-riscv64`) installed.
4. Launch the OS:
   * **macOS / Linux**: `run.sh`
   * **Windows**: `run.bat`

*Tip: You can simply expand qemu window for best experiense!*

---

## How to Build

### 1. Prerequisites
You need a RISC-V cross-compiler and the QEMU system emulator:
* **macOS**: `brew install riscv64-elf-gcc qemu`
* **Debian**: `sudo apt install gcc-riscv64-unknown-elf qemu-system-misc`
* **Other**: `You can just find instructions in the Net`

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

* **@luickk** & **[@CityAceE](https://github.com/CityAceE)** — Huge thanks for the original `qemu-ramfb` [driver](https://github.com/CityAceE/qemu-ramfb-riscv64-driver). Their work is a graphic base of mqos-rv.
* **[@lmemq](https://github.com/lmemq)** — Author of RISC-V mqos, [mqos-rv](https://github.com/lmemq/mqos-rv).
* **[@phoboslab](https://github.com/phoboslab)** - Thanks for [qoi.h](https://github.com/phoboslab/qoi), image format (Used under MIT)
* **NASA** - for providing wallpapers
* **Red Hat** — for the [Liberation Mono Font](https://github.com/liberationfonts/liberation-fonts) (Distributed under SIL Open Font License).
* **[@hubenchang0515](https://github.com/hubenchang0515)** — Thanks for the [ascii_font](https://github.com/hubenchang0515/ascii_font) generation tool (Used under MIT).
