
SYSCALLS = $(FPWR_DIR)/syscalls_minimal.c

ifeq ($(USE_POSIX_STYLE_IO), 1) 

ifneq ($(USE_MINSTDLIBS), 1) 
$(error to use posix style IO, USE_MINSTDLIBS must be set to 1)
endif

ifeq ($(USE_DRIVER_FAT_FILESYSTEM), 1)
$(error to use posix style IO, USE_DRIVER_FAT_FILESYSTEM must be set to 1)
endif

SYSCALLS = $(FPWR_DIR)/syscalls.c
CFLAGS += -I$(FPWR_DIR)
endif

SOURCE += $(SYSCALLS)
