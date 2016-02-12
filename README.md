# Toddler

## Breif history of Toddler

Toddler was originally a half-hobby and half-research project developed during summer and winter break when I was in college.
The original Toddler was designed for small multi-processor IA32 systems.
The most outstanding feature was the practical lock-free techniques used for syncronization.
However, it was overly designed for both hardware-related and regular OS parts.
Then it finally became unpractical to continue the development.
The final Toddler was able to create and run user processes and threads, as well as accepting keyboard inputs,
though a shell was not implemented or ported.

The new Toddler, on the other hand, is designed under a completely different goal.
Alghough it is still a hobby project, this time it aims to provide a fully usable microkernel and an OS environment.
The lock-free idea has been abandoned since it created too much unnecessary complexity.
