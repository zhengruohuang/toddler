#ifndef __COMMON_INCLUDE_ELF32__
#define __COMMON_INCLUDE_ELF32__


#include "common/include/data.h"


#define EI_NIDENT     16


/*
 * ELF32
 */
typedef u32     Elf32_Addr;
typedef u16     Elf32_Half;
typedef u32     Elf32_Off;
typedef s32     Elf32_Sword;
typedef u32     Elf32_Word;


struct elf32_header {
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

struct elf32_program {
    Elf32_Word      program_type;
    Elf32_Off       program_offset;
    Elf32_Addr      program_vaddr;
    Elf32_Addr      program_paddr;
    Elf32_Word      program_filesz;
    Elf32_Word      program_memsz;
    Elf32_Word      program_flags;
    Elf32_Word      program_align;
} packedstrut;


/*
 * ELF64
 */
typedef u64     Elf64_Addr;
typedef u16     Elf64_Half;
typedef s16     Elf64_SHalf;
typedef u64     Elf64_Off;
typedef s32     Elf64_Sword;
typedef u32     Elf64_Word;
typedef u64     Elf64_Xword;
typedef s64     Elf64_Sxword;

struct elf64_header {
    unsigned char   elf_ident[EI_NIDENT];
    Elf64_Half      elf_type;
    Elf64_Half      elf_machine;
    Elf64_Word      elf_version;
    Elf64_Addr      elf_entry;
    Elf64_Off       elf_phoff;
    Elf64_Off       elf_shoff;
    Elf64_Word      elf_flags;
    Elf64_Half      elf_ehsize;
    Elf64_Half      elf_phentsize;
    Elf64_Half      elf_phnum;
    Elf64_Half      elf_shentsize;
    Elf64_Half      elf_shnum;
    Elf64_Half      elf_shstrndx;
} packedstruct;

struct elf64_program {
    Elf64_Word      program_type;
    Elf64_Word      program_flags;
    Elf64_Off       program_offset;
    Elf64_Addr      program_vaddr;
    Elf64_Addr      program_paddr;
    Elf64_Xword     program_filesz;
    Elf64_Xword     program_memsz;
    Elf64_Xword     program_align;
} packedstruct;


/*
 * Native
 */
#if ARCH_WIDTH == 32
typedef struct elf32_header     elf_native_header_t;
typedef struct elf32_program    elf_native_program_t;
#else
typedef struct elf64_header     elf_native_header_t;
typedef struct elf64_program    elf_native_program_t;
#endif


#endif
