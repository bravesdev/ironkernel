# Iron Kernel (Basic Kernel)

A minimalist x86 32-bit monolithic kernel operating system written in C++ and Assembly. This project boots into Protected Mode using the Multiboot specification, providing a basic command-line interface (CLI) with manual hardware abstractions for the VGA text mode buffer and the 8042 PS/2 Keyboard Controller.

---

## System Architecture & Code Breakdown

Because this kernel runs bare-metal (directly on the hardware without an underlying operating system like Linux or Windows), it cannot rely on standard runtime libraries (`glibc`, `<iostream>`, `stdio.h`). Every single utility must be implemented from scratch by interfacing with hardware registers and memory-mapped I/O.

### 1. The Bootloader Hook (`loader.s`)
The PC BIOS cannot directly read or parse ELF binaries. It looks for a specific signature defined by the **Multiboot Specification**.
* **Magic Number (`0x1BADB002`)**: A mandatory constant that tells Multiboot-compliant bootloaders (like GRUB or QEMU's internal bootloader) that this binary is a valid kernel.
* **Stack Initialization**: Allocates `16384 bytes` (16KB) inside the uninitialized data section (`.bss`) and moves its address into the Stack Pointer register (`%esp`) before calling C++ code.
* **Handover**: Pushes boot information registers (`%eax`, `%ebx`) onto the stack and executes `call kmain`. If `kmain` returns, it disables interrupts (`cli`) and halts the CPU (`hlt`).

### 2. Hardware Micro-Abstractions (`kernel.hpp` & `kernel.cpp`)
This class encapsulates low-level x86 architecture primitives into an object-oriented structure via the `TerminalRootOS` class.
* **Memory-Mapped I/O (MMIO)**: The VGA Text Mode hardware maps its display frame buffer to the physical memory address `0xB8000`. Each character on the screen requires 2 bytes:
  * **Low Byte**: The ASCII character code.
  * **High Byte**: The attribute color code (e.g., `0x0F` represents bright white text on a black background).
* **Hardware Assembly Inlines**:
  * `inb` (*Input Byte*): Reads a byte from a hardware port configuration register using the `inb` assembly instruction.
  * `outb` (*Output Byte*): Writes a byte to a hardware port register using the `outb` assembly instruction.
* **Scrolling Latching**: When `cursor_y` reaches the height limit (`25`), the `scroll()` function copies memory rows `1-24` up to rows `0-23`, clearing the last line manually.
* **Hardware Cursor Control**: Uses ports `0x3D4` and `0x3D5` to interface with the VGA Controller CRT registers, shifting the physical blinking cursor on screen.
* **System Reboot**: Interfaced via the 8042 PS/2 Keyboard Controller status register (`0x64`). Sending a pulse value of `0xFE` triggers a pulse line pulling down the CPU's hardware reset line.

### 3. The Main Kernel Runtime & Shell (`main.cpp`)
Houses the main loop execution block `kmain`.
* **String Utilities (`strcmp`, `strstarts`)**: Reimplemented manually to allow matching parameters without standard library string definitions.
* **Scancode Translation (`scancode_to_ascii`)**: The keyboard controller sends raw matrix hardware scancodes (Set 1) whenever a physical switch is depressed. A lookup array maps these indexing scancodes into ASCII equivalents while discarding break codes (key release events flagged by bit `0x80`).
* **Command Shell Loop**: A sequential `while(1)` pipeline polling for scancode inputs. It fills up a 64-byte `cmd_buffer`, handling `\b` (backspace character updates) manually until an entry delimiter `\n` (Enter key) is evaluated against string tokens (`help`, `about`, `echo `, `clear`, `reboot`).

---

## Compilation & Execution Guide

### Prerequisites

#### For Linux (Ubuntu/Debian)
Install the standard GNU Compiler Collection (GCC) capable of cross-targeting i386 structures alongside the QEMU emulator:
```bash
sudo apt update
sudo apt install build-essential g++ qemu-system-x86

```

#### For Windows

1. **Compiler**: Download and install **Mingw-w64** (via MSYS2 or standalone package) to obtain `g++` and `ld`. Make sure its `bin` folder is added to your System Environment variables.
2. **Emulator**: Download and install [QEMU for Windows](https://www.google.com/search?q=https://www.qemu.org/download/%23windows). Add the installation directory (usually `C:\Program Files\qemu`) to your System Path variables.

---

### Building and Running on Linux

Open your terminal in the project directory and execute the following unified command sequence:

```bash
# 1. Assemble the boot loader stub
as --32 loader.s -o loader.o

# 2. Compile the implementation source files without standard host libraries
g++ -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -c kernel.cpp -o kernel.o
g++ -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -c main.cpp -o main.o

# 3. Link objects using the specified linker map script layout
ld -m elf_i386 -T link.ld -o kernel.bin loader.o kernel.o main.o

# 4. Boot up the compiled binary inside QEMU Emulation environment
qemu-system-i386 -kernel kernel.bin

```

---

### Building and Running on Windows

Open **Command Prompt (cmd)** or **PowerShell** inside the root folder of your project repository and run:

```cmd
:: 1. Assemble the boot loader stub
as --32 loader.s -o loader.o

:: 2. Compile kernel files removing host system dependencies and setting target configuration 
g++ -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -c kernel.cpp -o kernel.o
g++ -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -c main.cpp -o main.o

:: 3. Link objects matching the targeted i386 structural layout
ld -m i386pe -T link.ld -o kernel.bin loader.o kernel.o main.o

:: 4. Emulate the kernel image binary directly
qemu-system-i386.exe -kernel kernel.bin

```

> **Note on Windows Linking**: Depending on your specific distribution environment variables inside Mingw, if `elf_i386` is not recognized, changing the emulation flag to `-m i386pe` or omitting the `-m` configuration flag entirely while keeping `-T link.ld` will allow the link execution to finish successfully.

---

## Project Directory Map

```text
├── link.ld        # Linker script describing cross-compiled memory segment placement
├── loader.s       # Assembly entry point capturing hardware handover to Multiboot
├── kernel.hpp     # Class definition header file for the Terminal Root abstraction layer
├── kernel.cpp     # Explicit I/O and hardware frame control routine code block
└── main.cpp       # Core kernel shell engine housing string functions and main loop execution

```
---

## Author & Credits

* **BravesDev** - *Core Developer & Architecture Design* - [GitHub](https://github.com/BravesDev)

---

## License

This project is licensed under the **GNU GPL v3.0 License** - see the [LICENSE](LICENSE) file for complete legal details.
