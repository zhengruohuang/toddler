################################################################################
#
# Top level Makefile for Toddler
#
# Use Makefile.config to override any of the following variables
#
################################################################################

# Entry points
HALENTRY	= 0xFFF84000		# 4GB - 512KB + 16KB
KRNLENTRY	= 0xFFF01000		# 4GB - 1MB + 4KB
APPENTRY	= 0x8080000		# 128MB + 512KB
LIBCENTRY	= 0xC0001000

# Assembler, Compiler, Linker, and Flags
ASM		= nasm
ASMFLAGS	= -f elf

ASMBOOT		= nasm
ASMBOOTFLAGS	=

CC		= gcc
CFLAGS		= -c -g -fno-builtin -fno-stack-protector

LD		= ld
LDFLAGS		=

DASM		= ndisasm
DASMFLAGS	=

GENBIN		= objcopy
GENBINFLAGS	= -O binary

GENMAP		= objcopy
GENMAPFLAGS	= --only-keep-debug

STRIP		= strip
STRIPFLAGS	=

# Include the config file
include Makefile.config

# Directories
SRCDIR		= $(PROJDIR)/src
TARGETDIR	= $(PROJDIR)/target/$(ARCH)
DOCDIR		= $(PROJDIR)/doc
TOOLSDIR	= $(PROJDIR)/tools
VMDIR		= $(PROJDIR)/vm

# Including paths
CINC		= -I$(SRCDIR)/ -I$(SRCDIR)/arch/$(ARCH)/
ASMINC		= -i$(SRCDIR)/ -i$(SRCDIR)/arch/$(ARCH)/

# Exports and Unexports
export

# All phony targets
.PHONY: all build

# Default starting position
# Build everything
all: build startemu

# Compile and link the bianries
build: build_tools build_os coreimg floppyimg

# Build tools
build_tools:
	@mkdir -p $(TARGETDIR)/tools;			\
	cd $(TOOLSDIR);					\
	$(MAKE) --print-directory MAKEFLAGS= all;	\
	cd ../;

# Build OS
build_os:
	@mkdir -p $(TARGETDIR)/bin;			\
	mkdir -p $(TARGETDIR)/bin/boot;			\
	mkdir -p $(TARGETDIR)/img;			\
	mkdir -p $(TARGETDIR)/gdb;			\
	cd $(SRCDIR);					\
	$(MAKE) --print-directory MAKEFLAGS= all;	\
	cd ../;

# Clean
clean:
	cd $(SRCDIR);					\
	$(MAKE) --print-directory MAKEFLAGS= clean;	\
	cd ../;

# Build kernel image
coreimg:
	@$(TARGETDIR)/tools/coreimg			\
		$(TARGETDIR)/bin/tdlrcore.img		\
		$(TARGETDIR)/bin/tdlrhal.bin		\
		$(TARGETDIR)/bin/tdlrkrnl.bin		\
		$(TARGETDIR)/bin/tdlrsys.bin

# Build floppy image
floppyimg:
	@$(TARGETDIR)/tools/floppyimg			\
		$(TARGETDIR)/img/floppy.img		\
		$(TARGETDIR)/bin/boot/tdlrboot.fp	\
		$(TARGETDIR)/bin/boot/tdlrldr.bin	\
		$(TARGETDIR)/bin/tdlrcore.img

# Start a virtual machine or emulator
qemu:
	@qemu-system-i386 -m 128			\
		-fda $(TARGETDIR)/img/floppy.img	\
		-no-shutdown -no-reboot -no-kvm		\
		-smp cores=2,threads=2,sockets=2 # -numa node

bochs: build
	@cd $(VMDIR)/bochs;				\
	bochs -f bochsrc -q -log bochslog.txt -rc bochscmd;

vbox: build
	@VBoxManage startvm Toddler

startemu: qemu

src_lines:
	@echo Total number of lines:
	@find $(SRCDIR) -type f -exec cat {} + | wc -l
