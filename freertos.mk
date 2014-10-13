
FREERTOSDIR = $(FREERTOS_DIR)/Source

CFLAGS += -I$(FREERTOSDIR)/include

SOURCE += $(FREERTOSDIR)/list.c
SOURCE += $(FREERTOSDIR)/queue.c
SOURCE += $(FREERTOSDIR)/tasks.c
SOURCE += $(FREERTOSDIR)/portable/MemMang/heap_2.c

ifeq ($(FAMILY), STM32F1)
SOURCE += $(FREERTOSDIR)/portable/GCC/ARM_CM3/port.c
CFLAGS += -I$(FREERTOSDIR)/portable/GCC/ARM_CM3
else
ifeq ($(FAMILY), STM32F4)
SOURCE += $(FREERTOSDIR)/portable/GCC/ARM_CM4F/port.c
CFLAGS += -I$(FREERTOSDIR)/portable/GCC/ARM_CM4F
else
$(error the FAMILY specified '$(FAMILY)' is invalid, it is meant to be set in board.mk)
endif
endif