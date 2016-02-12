################################################################################
#
# Makefile for buliding Toddler image tools
#
################################################################################

# Targets
TARGETS	= $(TARGETDIR)/tools/floppyimg $(TARGETDIR)/tools/coreimg

# Phony target
.PHONY: all clean

# Make all
all: $(TARGETS)

# Clean
clean:
	rm -f $(TARGETS)

# Build tools
$(TARGETDIR)/tools/floppyimg : floppyimg.c
	$(CC) -m32 $(CINC) -o $@ $<

$(TARGETDIR)/tools/coreimg : coreimg.c
	$(CC) -m32 $(CINC) -o $@ $<
