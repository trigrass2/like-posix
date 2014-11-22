
SYSCALLS = $(LIKEPOSIX_DIR)/syscalls_minimal.c

USE_POSIX_STYLE_IO_VALUES = 0 1

ifeq ($(filter $(USE_POSIX_STYLE_IO),$(USE_POSIX_STYLE_IO_VALUES)), )
$(error USE_POSIX_STYLE_IO is not set. set to one of: $(USE_POSIX_STYLE_IO))
endif

CFLAGS += -DUSE_POSIX_STYLE_IO=$(USE_POSIX_STYLE_IO)

ifeq ($(USE_POSIX_STYLE_IO), 1) 

ifneq ($(USE_FREERTOS), 1) 
$(error to use posix style IO, USE_FREERTOS must be set to 1)
endif

ifneq ($(USE_MINSTDLIBS), 1) 
$(error to use posix style IO, USE_MINSTDLIBS must be set to 1)
endif

ifneq ($(USE_DRIVER_FAT_FILESYSTEM), 1)
$(error to use posix style IO, USE_DRIVER_FAT_FILESYSTEM must be set to 1)
endif

SYSCALLS = $(LIKEPOSIX_DIR)/syscalls.c
CFLAGS += -I$(LIKEPOSIX_DIR)
endif

# all system call that build independent of OS are included here
SYSCALLS += $(LIKEPOSIX_DIR)/time.c

SOURCE += $(SYSCALLS)

