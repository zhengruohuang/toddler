################################################################################
#                                                                              #
# Makefile for Toddler                                                         #
#                                                                              #
################################################################################

# Subdirectory for the root of sources to build
SRCSUBDIR	= .

# Target file
TARGET		= $(TARGETDIR)/bin/tdlrhal.bin

# Object directory
OBJDIR		= $(TARGETDIR)/obj/hal

# Sources
CSOURCES 	= $(wildcard *.c */*.c)
ASMSOURCES	= $(wildcard *.asm */*.asm)

# Custom build flags
CCUSTOMINC	=
ASMCUSTOMINC	=
CCUSTOMFLAGS	=
ASMCUSTOMFLAGS	=
LDCUSTOMFLAGS	= -Ttext $(HALENTRY)

# Phony target
.PHONY: all clean

# Make all
all: all_build

# Make clean
clean: clean_build

# Include the SUBDIR template
include $(SRCDIR)/Makefile.build
