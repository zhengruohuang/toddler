################################################################################
#
# Top level Makefile for Toddler
#
# Use Makefile.config to override any of the following variables
#
################################################################################

# Entry points
HALENTRY	= 0xffc08000		# 0xfff01000
ASMGRENTRY	= 0xff808000
APPENTRY	= 0xC0800000		# 3GB + 8MB
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

# Generate files for debugging
GENMAP		= objcopy
GENMAPFLAGS	= --only-keep-debug

STRIP		= strip
STRIPFLAGS	=

# Include the config file
include Makefile.config

# Directories
SRCDIR		= $(PROJDIR)/src
ALLTARGETDIR	= $(PROJDIR)/target
TARGETDIR	= $(PROJDIR)/target/$(ARCH)
DOCDIR		= $(PROJDIR)/doc
TOOLSDIR	= $(PROJDIR)/tools

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
	cd $(PROJDIR);

# Build OS
build_os:
	@mkdir -p $(TARGETDIR)/bin;			\
	mkdir -p $(TARGETDIR)/img;			\
	cd $(SRCDIR);					\
	$(MAKE) --print-directory MAKEFLAGS= all;	\
	cd $(PROJDIR);

# Clean
clean:
	@rm -rf $(ALLTARGETDIR)

#@cd $(SRCDIR);					
#$(MAKE) --print-directory MAKEFLAGS= clean;	
#cd $(PROJDIR);

# Build kernel image
coreimg:
	@$(TARGETDIR)/tools/coreimg			\
		$(TARGETDIR)/bin/tdlrkrnl.img		\
		$(TARGETDIR)/bin/tdlrhal.bin		\
		$(TARGETDIR)/bin/tdlrasmgr.bin		\
		$(TARGETDIR)/bin/tdlrbase.bin		\
		$(TARGETDIR)/bin/libc.bin

# Build floppy image
floppyimg:
	@$(TARGETDIR)/tools/floppyimg			\
		$(TARGETDIR)/img/floppy.img		\
		$(TARGETDIR)/bin/boot/tdlrboot.fp	\
		$(TARGETDIR)/bin/boot/tdlrldr.bin	\
		$(TARGETDIR)/bin/tdlrkrnl.img

# Start a virtual machine or emulator
qemu:
	@qemu-system-i386 -m 128			\
		-fda $(TARGETDIR)/img/floppy.img	\
		-no-shutdown -no-reboot -no-kvm		\
		-smp cores=2,threads=2,sockets=2 # -numa node -numa node # -numa node -numa node

bochs: build
	@cd $(VMDIR)/bochs;				\
	bochs -f bochsrc -q -log bochslog.txt -rc bochscmd;

vbox: build
	@VBoxManage startvm Toddler

startemu: qemu
