
USE_CONFPARSE_VALUES = 0 1
USE_LOGGER_VALUES = 0 1

ifeq ($(filter $(USE_CONFPARSE),$(USE_CONFPARSE_VALUES)), )
$(error USE_CONFPARSE is not set. set to one of: $(USE_CONFPARSE_VALUES))
endif

ifeq ($(filter $(USE_LOGGER),$(USE_LOGGER_VALUES)), )
$(error USE_LOGGER is not set. set to one of: $(USE_LOGGER_VALUES))
endif

CFLAGS += -DUSE_CONFPARSE=$(USE_CONFPARSE)
CFLAGS += -DUSE_LOGGER=$(USE_LOGGER)
CFLAGS += -DUSE_UDP_LOGGER=$(USE_UDP_LOGGER)
CFLAGS += -DUSE_LOGGER_TIMESTAMP=$(USE_LOGGER_TIMESTAMP)
CFLAGS += -I $(LIKEPOSIX_TOOLS_DIR)

# logger make be included even if not enabled
CFLAGS += -I $(LIKEPOSIX_TOOLS_DIR)/logger

# strutils may always be included
CFLAGS += -I $(LIKEPOSIX_TOOLS_DIR)/strutils
CFLAGS += -I $(LIKEPOSIX_TOOLS_DIR)/vfifo
SOURCE += $(LIKEPOSIX_TOOLS_DIR)/strutils/strutils.c
SOURCE += $(LIKEPOSIX_TOOLS_DIR)/vfifo/vfifo.c

ifeq ($(USE_LOGGER), 1)
SOURCE += $(LIKEPOSIX_TOOLS_DIR)/logger/logger.c
endif

ifeq ($(USE_CONFPARSE), 1)
SOURCE += $(LIKEPOSIX_TOOLS_DIR)/confparse/confparse.c
CFLAGS += -I $(LIKEPOSIX_TOOLS_DIR)/confparse
endif