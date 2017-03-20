
USE_JSMN_VALUES = 0 1

ifeq ($(filter $(USE_JSMN),$(USE_JSMN_VALUES)), )
$(error USE_JSMN is not set. set to one of: $(USE_JSMN_VALUES))
endif

CFLAGS += -DUSE_JSMN=$(USE_JSMN)

ifeq ($(USE_JSMN), 1)
CFLAGS += -I $(JSMNDIR)
CFLAGS += -I $(JSMN_EXTENSIONS_DIR)
SOURCE += $(JSMNDIR)/jsmn.c
SOURCE += $(JSMN_EXTENSIONS_DIR)/jsmn_extensions.c
endif
