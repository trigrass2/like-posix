
USE_MINSTDLIBS_VALUES = 0 1

ifeq ($(filter $(USE_MINSTDLIBS),$(USE_MINSTDLIBS_VALUES)), )
$(error USE_MINSTDLIBS is not set. set to one of: $(USE_MINSTDLIBS_VALUES))
endif

CFLAGS += -I $(MINSTDLIBSDIR)
CFLAGS += -DUSE_MINSTDLIBS=$(USE_MINSTDLIBS)

# strutils may always be included
SOURCE += $(MINSTDLIBSDIR)/strutils.c

ifeq ($(USE_MINSTDLIBS), 1)
SOURCE += $(MINSTDLIBSDIR)/minstring.c
SOURCE += $(MINSTDLIBSDIR)/minstdio.c
SOURCE += $(MINSTDLIBSDIR)/minstdlib.c
endif
