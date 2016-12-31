# Toddler

![Screenshot](https://cloud.githubusercontent.com/assets/17039006/21021636/5c26c944-bd47-11e6-809f-3e02bd932b64.png)

## Brief History

Toddler was originally a half-hobby and half-research project developed during summer and winter breaks when I was in college.
The original Toddler was designed for small-scale multi-processor IA32 systems.
The most outstanding feature was the practical lock-free techniques used for syncronization.
However, it was overly designed for both hardware-related and regular OS components.
Then it finally became impractical to continue the development.
The final Toddler was able to create and run user processes and threads, as well as accepting keyboard inputs,
though a shell was not implemented or ported.

The new Toddler, on the other hand, is designed with a completely different goal.
Alghough it is still a hobby project, it aims to provide a fully usable microkernel and a complete OS environment for multiple architectures and platforms.
The lock-free idea has been abandoned since it created too much unnecessary complexity.

## Building Toddler

Toddler has its own building system written in Python: tmake. tmake takes care of file dependancies and provides a series of primitives such as _compile_, _link_, _build_, and etc. tmake scripts (also in Python) then use the primitives to construct the building procedure.

Python is required for all targets; GCC and Binutils are required for each different target; NASM is required for x86 (ia32 and amd64) targets. Note that your toolchain may also require the corresponding libc6 development package. QEMU is also required if you want to test Toddler.

Once all the packages are installed, fetch the source code.
```bash
git clone https://github.com/zhengruohuang/toddler.git
cd toddler
```

Type ```./tmake build``` to build Toddler. Once done, it generates disk images in ```target/``` directory.  
If QEMU is installed for the target architecture, simply type ```./tmake qemu``` to start QEMU with default parameters.  
The two steps can be combined by typing ```./tmake all```, or simply ```./tmake```.

### Specifying Actions

tmake supports *actions*. To specify actions, use ```./tmake <actions>```.  
For example, ```./tmake clean build``` will clean up existing object and binary files, then start a new build.

### Specifying a Different Target

tmake supports multiple targets. In order to build for a specific target, use ```./tmake target=<arch-machine>[-suffix]```.  
Note that *arch* and *machine* fields are required by tmake, and *suffix* is optional. However, the actual implemention of a specific target may require the user supply a value for *suffix*.

For example, ```./tmake target=ia32-pc-bios``` will build Toddler for a BIOS-based IA32 PC system; ```./tmake target=armv7-rpi2``` will build Toddler for Raspberry Pi 2.  
Also note that you may not arbitrarily mix *arch* and *machine* fields. For example, ```target=ia32-rpi2``` is invalid. Invalid combinations of fields may fail to compile or even damage your device.

## Architecture

### Hardware Abstraction Layer

The hardware abstraction layer (HAL) provides an abstraction of each processor model and basic IO devices. It exports a series of functions and constants to kernel.
HAL is mapped to the highest 4MB of all processes except the kernel process.

### The kernel Process

Unlike conventional operating systems, Toddler's kernel is a *real* process. The kernel is not mapped to the address space of user processes.
However, the virtual address space layout of kernel is a bit different from other processes. The kernel has a *one-to-one* mapping thanks to the small size of HAL.
As a result, physical memory management is much easier and cleaner.

### The system Process

Although Toddler is a micro-kernel OS, being *many-server* might not be a good idea. Instead, many system-level functionalities are provided in a single server - system.
The system process implements universal resource manager (URS), user account manager (UAM), and several file systems.

### The driver Process

The driver process provides several essential device drivers, including keyboards, consoles, and disks.


## Ports

|Architecture|Width|Target Machine|Status|
|---|---|---|---|---|
|ia32|32|NetBurst-based PC|Active|
|armv7|32|QEMU Integrator/CP, QEMU RealView MP, Raspberry Pi 2|Active|
|mips32|32|QEMU Malta, Creator CI20|Active|
|ppc32|32|QEMU PREP, Mac Mini G4, PowerMac G4|Initial|
|sparcv8|32|SuperSPARC II|Planned|
|m68k|32|M68K|No Plan|
|amd64|64|Skylake-based PC|Planned|
|armv8|64|Raspberry Pi 3|Planned|
|ppc64|64|PowerMac G5|Planned|
|mips64|64|Loongson 3 Desktop|Planned|
|sparcv9|64|Sun UltraSPARC II Workstation|Planned|
|riscv|64|RISC V QEMU Emulator|Planned|
|ia64|64|Itaium 2|No Plan|
|alpha|64|ES40|No Plan|
|hppa|64|HP 9000 PA-RISC Workstation|No Plan|
|s390|64|S390 Emulator|No Plan|


## Development Plan

### Ports

|Time|Event|Arch|Target|Status|
|---|---|---|---|---|
|Feb. 2017|A working *modern* kernel and a simple working shell|ia32|Netbust PC|Active|
|Apr. 2017|A working 32-bit ARMv7 port|armv7|Raspberry Pi 2|Active|
|May 2017|SMP support for 32-bit ARMv7|armv7|Raspberry Pi 2|Planned|
|Aug. 2017|A working 32-bit PowerPC port|ppc32|Mac Mini G4|Planned|
|Sep. 2017|SMP support for 32-bit PowerPC|ppc32|PowerMac G4 Dual|Planned|
|Late 2017|64-bit Toddler|||Planned|

### Kernel

|Time|Event|Status|
|----|-----|------|
|Feb. 2017|Efficient memory management and sophisticated scheduling|Planned|
|Apr. 2017|Better SMP support|Planned|

### Building system

|Time|Event|Status|
|----|-----|------|
|Feb. 2017|Clean up the code, HD image builder|Planned|
|Apr. 2017|Parallel building|Planned|
