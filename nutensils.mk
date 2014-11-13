
CFLAGS += -I $(NUTENSILS_DIR)

USE_SOCK_UTILS_VALUES = 0 1

ifeq ($(filter $(USE_SOCK_UTILS),$(USE_SOCK_UTILS_VALUES)), )
$(error USE_SOCK_UTILS is not set. set to one of: $(USE_SOCK_UTILS_VALUES))
endif

CFLAGS += -DUSE_SOCK_UTILS=$(USE_SOCK_UTILS)

ifeq ($(USE_SOCK_UTILS), 1)
SOURCE += $(NUTENSILS_DIR)/socket/sock_utils.c
CFLAGS += -I $(NUTENSILS_DIR)/socket
endif

USE_HTTP_UTILS_VALUES = 0 1

ifeq ($(filter $(USE_HTTP_UTILS),$(USE_HTTP_UTILS_VALUES)), )
$(error USE_HTTP_UTILS is not set. set to one of: $(USE_HTTP_UTILS_VALUES))
endif

CFLAGS += -DUSE_HTTP_UTILS=$(USE_HTTP_UTILS)

ifeq ($(USE_HTTP_UTILS), 1)
SOURCE += $(NUTENSILS_DIR)/http/http_client.c
CFLAGS += -I $(NUTENSILS_DIR)/http
endif

USE_THREADED_SERVER_VALUES = 0 1

ifeq ($(filter $(USE_THREADED_SERVER),$(USE_THREADED_SERVER_VALUES)), )
$(error USE_THREADED_SERVER is not set. set to one of: $(USE_THREADED_SERVER_VALUES))
endif

CFLAGS += -DUSE_THREADED_SERVER=$(USE_THREADED_SERVER)

ifeq ($(USE_THREADED_SERVER), 1)
SOURCE += $(NUTENSILS_DIR)/threaded_server/threaded_server.c
CFLAGS += -I $(NUTENSILS_DIR)/threaded_server
endif


USE_SHELL_VALUES = 0 1

ifeq ($(filter $(USE_SHELL),$(USE_SHELL_VALUES)), )
$(error USE_SHELL is not set. set to one of: $(USE_SHELL_VALUES))
endif

CFLAGS += -DUSE_SHELL=$(USE_SHELL)

ifeq ($(USE_SHELL), 1)
SOURCE += $(NUTENSILS_DIR)/shell/shell.c
SOURCE += $(NUTENSILS_DIR)/shell/shell_command.c
CFLAGS += -I $(NUTENSILS_DIR)/shell
SOURCE += $(NUTENSILS_DIR)/shell/builtins/builtins.c
CFLAGS += -I $(NUTENSILS_DIR)/shell/builtins
SOURCE += $(NUTENSILS_DIR)/shell/filesystem_cmds/fs_cmds.c
CFLAGS += -I $(NUTENSILS_DIR)/shell/filesystem_cmds
endif

USE_HTTP_SERVER_VALUES = 0 1

ifeq ($(filter $(USE_HTTP_SERVER),$(USE_HTTP_SERVER_VALUES)), )
$(error USE_HTTP_SERVER is not set. set to one of: $(USE_HTTP_SERVER_VALUES))
endif

CFLAGS += -DUSE_HTTP_SERVER=$(USE_HTTP_SERVER)

ifeq ($(USE_HTTP_SERVER), 1)
SOURCE += $(NUTENSILS_DIR)/http/http_server.c
SOURCE += $(NUTENSILS_DIR)/http/http_api.c
endif
