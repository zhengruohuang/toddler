# Toddler

![Screenshot](https://cloud.githubusercontent.com/assets/17039006/21021636/5c26c944-bd47-11e6-809f-3e02bd932b64.png)

## Brief History

Toddler was originally a half-hobby and half-research project developed during summer and winter breaks when I was in college.
The original Toddler was designed for small-scale multi-processor IA32 systems.
The most outstanding feature was the practical lock-free techniques used for syncronization.
However, the old Toddler was overly designed for both hardware-related and regular OS components.
Then it finally became impractical to continue the development.
The final Toddler was able to create and run user processes and threads, as well as accepting keyboard inputs,
though a shell was not implemented or ported.

The new Toddler, on the other hand, is designed with a completely different goal.
Alghough it is still a hobby project, it aims to provide a fully usable microkernel and a complete OS environment for multiple architectures and platforms.
The lock-free idea is abandoned since it created too much unnecessary complexity.

## Building Toddler

Toddler has its own building system written in Python: _Tmake_. Tmake takes care of file dependancies and provides a series of primitives such as _compile_, _link_, _build_, and etc. Tmake scripts (also in Python) then use the primitives to construct the building procedure.

Python is required for all targets; GCC and Binutils are required for both host and target architectures; NASM is required for x86 (ia32 and amd64) targets. The default emulator (QEMU/SIMH/Ski) is also required if you want to test Toddler.

Once all the packages are installed, fetch the source code.
```bash
git clone https://github.com/zhengruohuang/toddler.git
cd toddler
```

Type ```./tmake build``` to build Toddler. Once done, it generates disk images in ```target/``` directory.  
If QEMU is installed for the target architecture, simply type ```./tmake qemu``` to start QEMU with default parameters.

The two steps can be combined by typing ```./tmake all```, or simply ```./tmake```.

### Specifying Actions

Tmake supports *actions*. To specify actions, use ```./tmake <actions>```.  
For example, ```./tmake clean build``` will clean up existing object and binary files, then start a new build.

### Specifying Targets

Tmake supports multiple targets. In order to build for a specific target, use ```./tmake target=<arch-machine>[-suffix]```.  
Note that *arch* and *machine* fields are required by Tmake, and *suffix* is optional. However, the actual implemention of a specific target may require the user supply a value for *suffix*.

For example, ```./tmake target=ia32-pc-bios``` will build Toddler for a BIOS-based IA32 PC system; ```./tmake target=armv7-rpi2``` will build Toddler for Raspberry Pi 2.

Also note that you may not arbitrarily mix *arch* and *machine* fields. For example, ```target=ia32-rpi2``` is invalid.  
Invalid combinations of fields may fail to compile or even damage your device.

## Architecture

### Hardware Abstraction Layer

The hardware abstraction layer (HAL) provides an abstraction of each processor model and basic IO devices. It exports a series of functions and constants to kernel.
HAL is mapped to the highest 4MB of all processes including the kernel process.

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
|---|---|---|---|
|ia32|32|NetBurst-based PC|Current|
|armv7|32|Raspberry Pi 2, QEMU raspi2|Current|
|mips32|32|Creator CI20, QEMU malta|Current|
|ppc32|32|Mac Mini G4, PowerMac G4, QEMU g3beige, QEMU mac99|Current|
|sparcv8|32|QEMU sum4m SPARCstation 10, QEMU Leon-3|Active|
|riscv32|32|Spike|Planned|
|m68k|32|QEMU mcf5208evb|Planned|
|sh4|32|QEMU shix|Planned|
|vax|32|SIMH VAX|Planned|
|or1k|32|QEMU or1k-sim|Planned|
|amd64|64|Skylake-based PC|Planned|
|armv8|64|Raspberry Pi 3, QEMU virt|Active|
|ppc64|64|PowerMac G5, QEMU mac99|Planned|
|mips64|64|Loongson 3 Desktop, QEMU malta|Current|
|sparcv9|64|QEMU sun4u|Planned|
|alpha|64|QEMU clipper|Initial|
|riscv64|64|Spike|Planned|
|s390|64|QEMU s390x|Planned|
|ia64|64|Ski|Planned|
|hppa|64|HP 9000 PA-RISC Workstation, QEMU hppa-generic|Planned|
