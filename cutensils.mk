
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
CFLAGS += -I $(CUTENSILS_DIR)

# logger make be included even if not enabled
CFLAGS += -I $(CUTENSILS_DIR)/logger

ifeq ($(USE_LOGGER), 1)
SOURCE += $(CUTENSILS_DIR)/logger/logger.c
endif

ifeq ($(USE_CONFPARSE), 1)
SOURCE += $(CUTENSILS_DIR)/confparse/confparse.c
CFLAGS += -I $(CUTENSILS_DIR)/confparse
endif