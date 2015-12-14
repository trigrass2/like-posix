
USE_DRIVER_FAT_FILESYSTEM_VALUES = 0 1

ifeq ($(filter $(USE_DRIVER_FAT_FILESYSTEM),$(USE_DRIVER_FAT_FILESYSTEM_VALUES)), )
$(error USE_DRIVER_FAT_FILESYSTEM is not set. set to one of: $(USE_DRIVER_FAT_FILESYSTEM))
endif

CFLAGS += -DUSE_DRIVER_FAT_FILESYSTEM=$(USE_DRIVER_FAT_FILESYSTEM)

## FAT Filesystem
ifeq ($(USE_DRIVER_FAT_FILESYSTEM), 1)

ifeq ($(USE_DRIVER_SDCARD), 1) 
fatfs_driver_enabled = 1
endif
ifeq ($(USE_DRIVER_RAMDISK), 1) 
fatfs_driver_enabled = 1
endif

ifneq ($(fatfs_driver_enabled), 1) 
$(error to use FATFS, set USE_DRIVER_SDCARD=1 and/or USE_DRIVER_RAMDISK=1. if SPI sdcard mode is set in boardname.bsp/sdcard_config.h, then set USE_DRIVER_SPI=1)
endif

SOURCE += $(FATFS_DIR)/core/option/syscall.c
SOURCE += $(FATFS_DIR)/core/option/ccsbcs.c
SOURCE += $(FATFS_DIR)/core/ff.c

CFLAGS += -I$(FATFS_DIR)/core

CFLAGS += -D _FS_LOCK=$(_FS_LOCK)
CFLAGS += -D _FS_TINY=$(_FS_TINY)
CFLAGS += -D _FS_READONLY=$(_FS_READONLY)
endif