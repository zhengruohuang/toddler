# Toddler

## Breif history of Toddler

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


## Architecture

## Ports

|Architecture|Bits|Platform|Status|
|IA32|32|PC|Active|
|PowerPC|32|Mac Mini G4, PowerMac G4|Initial|
|ARMv7|32|Raspberry 2|Initial|
|AMD64|64|PC|Planned|
|PowerPC|64|PowerMac G5|Planned|
|ARMv8|64|Raspberry 3|Planned|
