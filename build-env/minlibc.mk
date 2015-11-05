
USE_MINLIBC_VALUES = 0 1

ifeq ($(filter $(USE_MINLIBC),$(USE_MINLIBC_VALUES)), )
$(error USE_MINLIBC is not set. set to one of: $(USE_MINLIBC_VALUES))
endif

CFLAGS += -I $(MINLIBCDIR)
CFLAGS += -DUSE_MINLIBC=$(USE_MINLIBC)


ifeq ($(USE_MINLIBC), 1)
SOURCE += $(MINLIBCDIR)/string.c
SOURCE += $(MINLIBCDIR)/stdio.c
SOURCE += $(MINLIBCDIR)/stdlib.c
SOURCE += $(MINLIBCDIR)/unistd.c
SOURCE += $(MINLIBCDIR)/termios.c
endif

# all system call that build independent of OS are included here
SOURCE += $(MINLIBCDIR)/time.c
SOURCE += $(MINLIBCDIR)/libgen.c
SOURCE += $(MINLIBCDIR)/dirent.c
SOURCE += $(MINLIBCDIR)/stat.c