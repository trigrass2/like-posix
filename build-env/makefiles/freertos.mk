
USE_FREERTOS_VALUES = 0 1

ifeq ($(filter $(USE_FREERTOS),$(USE_FREERTOS_VALUES)), )
$(error USE_FREERTOS is not set. set to one of: $(USE_FREERTOS_VALUES))
endif

CFLAGS += -DUSE_FREERTOS=$(USE_FREERTOS)

ifeq ($(USE_FREERTOS), 1)

FREERTOSDIR = $(FREERTOS_DIR)/Source

# for freertos_version.h
CFLAGS += -I$(FREERTOSDIR)/..
CFLAGS += -I$(FREERTOSDIR)/include

SOURCE += $(FREERTOSDIR)/list.c
SOURCE += $(FREERTOSDIR)/queue.c
SOURCE += $(FREERTOSDIR)/tasks.c
SOURCE += $(FREERTOSDIR)/timers_.c
SOURCE += $(FREERTOSDIR)/portable/MemMang/heap_2.c

ifeq ($(FAMILY), STM32F1)
SOURCE += $(FREERTOSDIR)/portable/GCC/ARM_CM3/port.c
CFLAGS += -I$(FREERTOSDIR)/portable/GCC/ARM_CM3
else
ifeq ($(FAMILY), STM32F4)
SOURCE += $(FREERTOSDIR)/portable/GCC/ARM_CM4F/port.c
CFLAGS += -I$(FREERTOSDIR)/portable/GCC/ARM_CM4F
SOURCE += $(FREERTOSDIR)/../heap_ccram.c
else
$(error the FAMILY specified '$(FAMILY)' is invalid, it is meant to be set in board.mk)
endif
endif

endif
