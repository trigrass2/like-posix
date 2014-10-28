
USE_SOCK_UTILS_VALUES = 0 1

ifeq ($(filter $(USE_SOCK_UTILS),$(USE_SOCK_UTILS_VALUES)), )
$(error USE_SOCK_UTILS is not set. set to one of: $(USE_SOCK_UTILS_VALUES))
endif

CFLAGS += -DUSE_SOCK_UTILS=$(USE_SOCK_UTILS)
CFLAGS += -I $(NUTENSILS_DIR)

ifeq ($(USE_SOCK_UTILS), 1)
SOURCE += $(NUTENSILS_DIR)/socket/sock_utils.c
CFLAGS += -I $(NUTENSILS_DIR)/socket
endif

USE_HTTP_UTILS_VALUES = 0 1

ifeq ($(filter $(USE_HTTP_UTILS),$(USE_HTTP_UTILS_VALUES)), )
$(error USE_HTTP_UTILS is not set. set to one of: $(USE_HTTP_UTILS_VALUES))
endif

CFLAGS += -DUSE_HTTP_UTILS=$(USE_HTTP_UTILS)
CFLAGS += -I $(NUTENSILS_DIR)

ifeq ($(USE_HTTP_UTILS), 1)
SOURCE += $(NUTENSILS_DIR)/http/http_client.c
CFLAGS += -I $(NUTENSILS_DIR)/http
endif
