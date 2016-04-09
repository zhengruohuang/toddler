################################################################################
#
# Top level Makefile for Toddler
#
################################################################################


# Target CPU architecture.
# Supported values
# 	unknown - same as host
# 	ia32
# 	ppc
ARCH		= ia32


# Project dir
PROJDIR		= $(CURDIR)
SRCDIR		= $(PROJDIR)/src
TARGETDIR	= $(PROJDIR)/target/$(ARCH)
DOCDIR		= $(PROJDIR)/doc
TOOLSDIR	= $(PROJDIR)/tools
VMDIR		= $(PROJDIR)/vm


# Assembler, Compiler, Linker, and Flags
ASM		= nasm
ASMFLAGS	= -f elf

ASMBOOT		= nasm
ASMBOOTFLAGS	=

CC		= gcc
CFLAGS		= -c -g -fno-builtin -fno-stack-protector -O3

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

# Including paths
CINC		= -I$(SRCDIR)/ -I$(SRCDIR)/arch/$(ARCH)/
ASMINC		= -i$(SRCDIR)/ -i$(SRCDIR)/arch/$(ARCH)/

# Treat warnings as errors
CFLAGS		+= -Wall -Wpointer-arith -Wcast-align -Wno-int-to-pointer-cast
CFLAGS		+= -Werror


# Arch specific makefile
include $(PROJDIR)/src/arch/$(ARCH)/Makefile.config


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
