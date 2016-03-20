################################################################################
#                                                                              #
# Makefile for Toddler                                                         #
#                                                                              #
################################################################################

# Subdirectory for the root of sources to build
SRCSUBDIR	= .

# Target file
TARGET		= $(TARGETDIR)/bin/tdlrsys.bin

# Object directory
OBJDIR		= $(TARGETDIR)/obj/system

# Sources
CSOURCES 	= $(wildcard *.c */*.c)
ASMSOURCES	= $(wildcard *.asm */*.asm)

# Custom build flags
CCUSTOMFLAGS	=
ASMCUSTOMFLAGS	=
LDCUSTOMFLAGS	= -Ttext $(APPENTRY)

# Phony target
.PHONY: all clean

# Make all
all: all_build

# Make clean
clean: clean_build

# Include the SUBDIR template
include $(SRCDIR)/Makefile.build
