
USE_DRIVER_FAT_FILESYSTEM_VALUES = 0 1

ifeq ($(filter $(USE_DRIVER_FAT_FILESYSTEM),$(USE_DRIVER_FAT_FILESYSTEM_VALUES)), )
$(error USE_DRIVER_FAT_FILESYSTEM is not set. set to one of: $(USE_DRIVER_FAT_FILESYSTEM))
endif

CFLAGS += -DUSE_DRIVER_FAT_FILESYSTEM=$(USE_DRIVER_FAT_FILESYSTEM)

## FAT Filesystem
ifeq ($(USE_DRIVER_FAT_FILESYSTEM), 1)

ifneq ($(USE_DRIVER_SDCARD), 1) 
$(error to use FATFS, USE_DRIVER_SDCARD must be set to 1)
endif

SOURCE += $(FATFS_DIR)/core/option/syscall.c
SOURCE += $(FATFS_DIR)/core/option/ccsbcs.c
SOURCE += $(FATFS_DIR)/core/ff.c

SOURCE += $(FATFS_DIR)/diskio_stm32.c

CFLAGS += -I$(FATFS_DIR)/core

CFLAGS += -D _FS_LOCK=$(_FS_LOCK)
CFLAGS += -D _FS_TINY=$(_FS_TINY)
CFLAGS += -D _FS_READONLY=$(_FS_READONLY)
endif