#ifndef __COMMON_INCLUDE_ELF32__
#define __COMMON_INCLUDE_ELF32__


#include "common/include/data.h"


#define EI_NIDENT     16


typedef u32     Elf32_Addr;
typedef u16     Elf32_Half;
typedef u32     Elf32_Off;
typedef u32     Elf32_Sword;
typedef u32     Elf32_Word;


struct elf32_elf_header {
    u8              elf_ident[EI_NIDENT];
    Elf32_Half      elf_type;
    Elf32_Half      elf_machine;
    Elf32_Word      elf_version;
    Elf32_Addr      elf_entry;
    Elf32_Off       elf_phoff;
    Elf32_Off       elf_shoff;
    Elf32_Word      elf_flags;
    Elf32_Half      elf_ehsize;
    Elf32_Half      elf_phentsize;
    Elf32_Half      elf_phnum;
    Elf32_Half      elf_shentsize;
    Elf32_Half      elf_shnum;
    Elf32_Half      elf_shstrndx;
} packedstruct;

struct elf32_program_header {
    Elf32_Word      program_type;
    Elf32_Off       program_offset;
    Elf32_Addr      program_vaddr;
    Elf32_Addr      program_paddr;
    Elf32_Word      program_filesz;
    Elf32_Word      program_memsz;
    Elf32_Word      program_flags;
    Elf32_Word      program_align;
} packedstrut;


#endif
