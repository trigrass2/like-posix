
USE_MINLIBC_VALUES = 0 1
USE_DRIVER_FAT_FILESYSTEM_VALUES = 0 1
USE_LIKEPOSIX_VALUES = 0 1

ifeq ($(filter $(USE_MINLIBC),$(USE_MINLIBC_VALUES)), )
$(error USE_MINLIBC is not set. set to one of: $(USE_MINLIBC_VALUES))
endif

ifeq ($(filter $(USE_DRIVER_FAT_FILESYSTEM),$(USE_DRIVER_FAT_FILESYSTEM_VALUES)), )
$(error USE_DRIVER_FAT_FILESYSTEM is not set. set to one of: $(USE_DRIVER_FAT_FILESYSTEM_VALUES))
endif

ifeq ($(filter $(USE_LIKEPOSIX),$(USE_LIKEPOSIX_VALUES)), )
$(error USE_LIKEPOSIX is not set. set to one of: $(USE_LIKEPOSIX_VALUES))
endif

CFLAGS += -I $(MINLIBCDIR)
CFLAGS += -DUSE_MINLIBC=$(USE_MINLIBC)


ifeq ($(USE_MINLIBC), 1)
SOURCE += $(MINLIBCDIR)/string.c
SOURCE += $(MINLIBCDIR)/stdio.c
SOURCE += $(MINLIBCDIR)/stdlib.c
ifeq ($(USE_LIKEPOSIX), 1)
SOURCE += $(MINLIBCDIR)/termios.c
SOURCE += $(MINLIBCDIR)/unistd.c
endif
ifeq ($(USE_DRIVER_FAT_FILESYSTEM), 1)
SOURCE += $(MINLIBCDIR)/dirent.c
SOURCE += $(MINLIBCDIR)/stat.c
endif
endif

# all system call that build independent of OS are included here
SOURCE += $(MINLIBCDIR)/time.c
SOURCE += $(MINLIBCDIR)/libgen.c