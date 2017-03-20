
#default to using minimal syscalls
SYSCALLS = $(LIKEPOSIX_CORE_DIR)/syscalls_minimal.c

USE_LIKEPOSIX_VALUES = 0 1

ifeq ($(filter $(USE_LIKEPOSIX),$(USE_LIKEPOSIX_VALUES)), )
$(error USE_LIKEPOSIX is not set. set to one of: $(USE_LIKEPOSIX))
endif

CFLAGS += -DUSE_LIKEPOSIX=$(USE_LIKEPOSIX)

ifeq ($(USE_LIKEPOSIX), 1) 

ifneq ($(USE_FREERTOS), 1) 
$(error to use posix style IO, USE_FREERTOS must be set to 1)
endif

ifneq ($(USE_MINLIBC), 1) 
$(error to use posix style IO, USE_MINLIBC must be set to 1)
endif

ifneq ($(USE_DRIVER_FAT_FILESYSTEM), 1)
$(error to use posix style IO, USE_DRIVER_FAT_FILESYSTEM must be set to 1)
endif



# override minimal syscalls with full implementation
SYSCALLS = $(LIKEPOSIX_CORE_DIR)/syscalls.c
SYSCALLS += $(LIKEPOSIX_CORE_DIR)/stdlib_impl.c
endif

ifeq ($(USE_FREERTOS), 1) 
SOURCE += $(LIKEPOSIX_CORE_DIR)/stackoverflow.c
endif

SYSCALLS += $(LIKEPOSIX_CORE_DIR)/asserts.c
SYSCALLS += $(LIKEPOSIX_CORE_DIR)/likeposix_init.c

CFLAGS += -I$(LIKEPOSIX_CORE_DIR)

SOURCE += $(SYSCALLS)

