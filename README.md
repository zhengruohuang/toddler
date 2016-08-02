# Toddler

## Brief History of Toddler

Toddler was originally a half-hobby and half-research project developed during summer and winter breaks when I was in college.
The original Toddler was designed for small-scale multi-processor IA32 systems.
The most outstanding feature was the practical lock-free techniques used for syncronization.
However, it was overly designed for both hardware-related and regular OS components.
Then it finally became unpractical to continue the development.
The final Toddler was able to create and run user processes and threads, as well as accepting keyboard inputs,
though a shell was not implemented or ported.

The new Toddler, on the other hand, is designed with a completely different goal.
Alghough it is still a hobby project, it aims to provide a fully usable microkernel and a complete OS environment for multiple architectures and platforms.
The lock-free idea has been abandoned since it created too much unnecessary complexity.

## Building Toddler

Toddler has its own building system: tmake. tmake takes care of file dependancies automatically and provides a series of primitives such as compile_files, link_files, build_file, and etc. tmake scripts (also in Python) then use the primitives to construct the build procedure.

Toddler does not have many external dependancies. GCC, Binutils, and Python are required for all targets, NASM is required for x86 (ia32 and amd64) targets. Note that your toolchain may also require libc6-dev for the corresponding target. To test Toddler, QEMU is also required.

Once all the packages are installed, go into toddler's directory, then type
./tmake build
This will start the building process. Once done, it generates disk images in /target directory.

Once QEMU is installed, simply type
./tmake qemu
This will start QEMU with default parameters.

These to steps can be combined by typing
./tmake all
or simply
./tmake
This will build then test.

## Architecture

### Hardware Abstraction Layer

### The kernel Process

### The system Process

### The driver Process


## Ports

|Architecture|Bits|Platform|Status|
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
