
USE_FREERTOS_VALUES = 0 1
USE_PTHREADS_VALUES = 0 1

ifeq ($(filter $(USE_FREERTOS),$(USE_FREERTOS_VALUES)), )
$(error USE_FREERTOS is not set. set to one of: $(USE_FREERTOS_VALUES))
endif

ifeq ($(filter $(USE_PTHREADS),$(USE_PTHREADS_VALUES)), )
$(error USE_PTHREADS is not set. set to one of: $(USE_PTHREADS_VALUES))
endif

CFLAGS += -DUSE_PTHREADS=$(USE_PTHREADS)

ifeq ($(USE_PTHREADS), 1)
CFLAGS += -I $(PTHREADS_DIR)
SOURCE += $(PTHREADS_DIR)/pthread.c
endif
