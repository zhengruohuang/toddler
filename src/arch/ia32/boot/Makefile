################################################################################
#                                                                              #
# Makefile for Toddler                                                         #
#                                                                              #
################################################################################

# Targets
TARGETS	= $(TARGETDIR)/bin/boot/tdlrboot.fp

#		$(TARGETDIR)/bin/boot/tdlrboot.hd \
#		$(TARGETDIR)/bin/boot/tdlrboot.cd

# Phony target
.PHONY: all clean

# Make all
all: $(TARGETS)

# Make clean
clean:
	rm -f $(TARGETS)

# Build Boot
$(TARGETDIR)/bin/boot/tdlrboot.fp : floppy/floppy.c \
					floppy/floppy.c \
					floppy/boot.asm
	$(ASMBOOT) $(ASMINC) $(ASMBOOTFLAGS) -o $@.boot floppy/boot.asm
	$(CC) $(CFLAGS) $(CINC) -fno-tree-loop-optimize -c -o $@.o $<
	$(LD) $(LDFLAGS) -s -T floppy/floppy.ld -o $@.elf $@.o
	$(GENBIN) $(GENBINFLAGS) $@.elf $@.floppy
	rm $@.elf $@.o
	cat $@.boot $@.floppy > $@
	rm $@.boot $@.floppy

$(TARGETDIR)/bin/boot/tdlrboot.hd : hd/hd.c \
					hd/hd.h \
					hd/fat.h \
					hd/ntfs.h \
					hd/boot.asm \
					hd/hd.ld
	$(ASMBOOT) $(ASMINC) $(ASMBOOTFLAGS) -o $@.boot hd/boot.asm
	$(CC) $(CFLAGS) $(CINC) -fno-tree-loop-optimize -c -o $@.o $<
	$(LD) $(LDFLAGS) -s -T hd/hd.ld -o $@.elf $@.o
	$(GENBIN) $(GENBINFLAGS) $@.elf $@.hd
	rm $@.elf $@.o
	cat $@.boot $@.hd > $@
	rm $@.boot $@.hd

$(TARGETDIR)/bin/boot/tdlrboot.cd : cd/cd.c \
					cd/cd.c \
					cd/boot.asm
	$(ASMBOOT) $(ASMINC) $(ASMBOOTFLAGS) -o $@.boot cd/boot.asm
	$(CC) $(CFLAGS) $(CINC) -fno-tree-loop-optimize -c -o $@.o $<
	$(LD) $(LDFLAGS) -s -T cd/cd.ld -o $@.elf $@.o
	$(GENBIN) $(GENBINFLAGS) $@.elf $@.cd
	rm $@.elf $@.o
	cat $@.boot $@.cd > $@
	rm $@.boot $@.cd
