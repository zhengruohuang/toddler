################################################################################
#                                                                              #
# Makefile for the Toddler                                                     #
#                                                                              #
################################################################################

# Targets
TARGETS	= $(TARGETDIR)/bin/boot/tdlrldr.bin

# Phony target
.PHONY: all clean

# Make all
all: $(TARGETS)

# Make clean
clean:
	rm -f $(TARGETS)

# Build Boot
$(TARGETDIR)/bin/boot/tdlrldr.bin : start.asm \
					loader.h \
					real.c real.ld \
					protected.c protected.ld
	$(ASMBOOT) $(ASMINC) $(ASMBOOTFLAGS) -o $@.start start.asm
	$(CC) $(CFLAGS) $(CINC) -fno-tree-loop-optimize -c -o $@.real.o real.c
	$(LD) $(LDFLAGS) -s -T real.ld -o $@.real.elf $@.real.o
	$(GENBIN) $(GENBINFLAGS) $@.real.elf $@.real
	rm $@.real.elf $@.real.o
	$(CC) $(CFLAGS) $(CINC) -fno-tree-loop-optimize -c -o $@.protected.o protected.c
	$(LD) $(LDFLAGS) -s -T protected.ld -o $@.protected.elf $@.protected.o
	$(GENBIN) $(GENBINFLAGS) $@.protected.elf $@.protected
	rm $@.protected.elf $@.protected.o
	cat $@.start $@.real $@.protected > $@
	rm $@.start $@.real $@.protected
