################################################################################
#                                                                              #
# Makefile for Toddler                                                         #
#                                                                              #
################################################################################

# Subdirectory for the root of sources to build
SRCSUBDIR	= .

# Target file
TARGET		= $(TARGETDIR)/bin/tdlrkrnl.bin

# Object directory
OBJDIR		= $(TARGETDIR)/obj/kernel

# Sources
CSOURCES 	= $(wildcard *.c */*.c)
ASMSOURCES	= $(wildcard *.asm */*.asm)

# Custom build flags
CCUSTOMFLAGS	=
ASMCUSTOMFLAGS	=
LDCUSTOMFLAGS	= -Ttext $(KRNLENTRY)

# Phony target
.PHONY: all clean

# Make all
all: all_build

# Make clean
clean: clean_build

# Include the SUBDIR template
include $(SRCDIR)/Makefile.build
