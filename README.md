# Toddler

## Brief History of Toddler

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

Python is required for all targets; GCC and Binutils are required for each different target; NASM is required for x86 (ia32 and amd64) targets. Note that your toolchain may also require the corresponding libc6-dev. QEMU is also required if you want to test Toddler.

Once all the packages are installed, fetch the source code then go into the ```toddler/``` directory.
```bash
git clone https://github.com/zhengruohuang/toddler.git
cd toddler
```

Type ```./tmake build``` to build Toddler. Once done, it generates disk images in the ```target/``` directory.

If QEMU is installed for the target architecture, simply type ```./tmake qemu``` to start QEMU with default parameters.

The two steps can be combined by typing ```./tmake all```, or simply ```./tmake```.


## Architecture

### Hardware Abstraction Layer

### The kernel Process

### The system Process

### The driver Process


## Ports

|Architecture|Width|Platform|Status|
|---|---|---|---|---|
|ia32|32|NetBurst-based PC|Active|
|ppc32|32|Mac Mini G4, PowerMac G4|Initial|
|armv7|32|Raspberry Pi 2|Initial|
|mips32|32|MIPS 32|Planned|
|sparcv8|32|SuperSPARC II|Planned|
|m68k|32|M68K|No Plan|
|amd64|64|Skylake-based PC|Planned|
|ppc64|64|PowerMac G5|Planned|
|armv8|64|Raspberry Pi 3|Planned|
|mips64|64|MIPS 64|Planned|
|sparcv9|64|Sun UltraSPARC II Workstation|Planned|
|riscv|64|RISC V|Planned|
|ia64|64|Itaium 2|No Plan|
|alpha|64|ES40|No Plan|
|hppa|64|HP RA-RISC|No Plan|
|s390|64|S390|No Plan|


## Development Plan

### The Kernel

|Time|Event|Architecture|Target|Status|
|---|---|---|---|---|
|Oct. 2016|A working *modern* kernel and a simple working shell|ia32|Netbust PC|Active|
|Nov. 2016|A working 32-bit PowerPC port|ppc32|Mac Mini G4|Planned|
|Dec. 2016|SMP support for 32-bit PowerPC|ppc32|PowerMac G4 Dual|Planned|
|Jan. 2017|A working 32-bit ARMv7 port|armv7|Raspberry Pi 2|Planned|
|Feb. 2017|SMP support for 32-bit ARMv7|armv7|Raspberry Pi 2|Planned|
|Later|64-bit Toddler|||Planned|
