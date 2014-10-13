
DEVICEDIR = $(DEVICE_SUPPORT_DIR)/device

## supported device FAMILIES
FAMILIES = STM32F1 STM32F4
FAMILY_FLAGS = -D STM32F1=1 -D STM32F4=4
## supported DEVICES
DEVICES = stm32f103ve stm32f407ve stm32f407vg

## test FAMILY againt FAMILIES
ifeq ($(filter $(FAMILY),$(FAMILIES)), )
$(error FAMILY '$(FAMILY)' not supported. supported FAMILIES: $(FAMILIES))
endif

## Configure FAMILY
ifeq ($(FAMILY), STM32F1)
FREERTOS_PORT = GCC_ARMCM3
CPU_FLAGS = -mthumb -mcpu=cortex-m3
FPU_FLAGS =
CHIPSUPPORTDIR = $(DEVICEDIR)/CM3
CHIPSUPPORT_MK = stm32f10x.mk
endif
ifeq ($(FAMILY), STM32F4)
FREERTOS_PORT = GCC_ARMCM4
CPU_FLAGS = -mthumb -mcpu=cortex-m4
FPU_FLAGS += -D __FPU_PRESENT=1 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fno-strict-aliasing -Wdouble-promotion -fsingle-precision-constant
CHIPSUPPORT_MK = stm32f4xx.mk
CHIPSUPPORTDIR = $(DEVICEDIR)/CM4
endif

## test DEVICE against DEVICES
ifeq ($(filter $(DEVICE),$(DEVICES)), )
$(error device '$(DEVICE)' not supported. supported devices: $(DEVICES))
endif

## configure DEVICE
ifeq ($(DEVICE), stm32f103ve)
LINKER_FILE = stm32f103ve.ld
STARTUP_FILE = startup_stm32f10x_hd.c
DENSITY = STM32F10X_HD
endif
ifeq ($(DEVICE), stm32f407ve)
LINKER_FILE = stm32f407ve.ld
STARTUP_FILE = startup_stm32f4xx.c
DENSITY = STM32F4XX
endif
ifeq ($(DEVICE), stm32f407vg)
LINKER_FILE = stm32f407vg.ld
STARTUP_FILE = startup_stm32f4xx.c
DENSITY = STM32F4XX
endif

STARTUP_SOURCE = $(CHIPSUPPORTDIR)/$(STARTUP_FILE)
LDSCRIPT = $(CHIPSUPPORTDIR)/$(LINKER_FILE)

## configure  CFLAGS
CFLAGS += $(FAMILY_FLAGS)
CFLAGS += $(CPU_FLAGS) $(FPU_FLAGS)
CFLAGS += -D $(FREERTOS_PORT)
CFLAGS += -D $(DENSITY)
CFLAGS += -I $(CHIPSUPPORTDIR)/std_periph_drivers/inc
CFLAGS += -I $(CHIPSUPPORTDIR)/ethernet
CFLAGS += -I $(DEVICEDIR)
CFLAGS += -I $(CHIPSUPPORTDIR)

include $(CHIPSUPPORTDIR)/$(CHIPSUPPORT_MK)

