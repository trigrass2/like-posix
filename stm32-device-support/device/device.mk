
DEVICEDIR = $(DEVICE_SUPPORT_DIR)/device

## supported DEVICES

STM32F1_DEVICES = STM32F100xB STM32F100xE STM32F101x6 STM32F101xB STM32F101xE STM32F101xG STM32F102x6 STM32F102xB STM32F103x6 STM32F103xB STM32F103xE STM32F103xG STM32F105xC STM32F107xC
STM32F4_DEVICES = STM32F405xx STM32F415xx STM32F407xx STM32F417xx STM32F427xx STM32F437xx STM32F429xx STM32F439xx STM32F401xC STM32F401xE STM32F410Tx STM32F410Cx STM32F410Rx STM32F411xE STM32F446xx STM32F469xx STM32F479xx stm32f479xx.h

STM32F1_DEVICES = STM32F103xE STM32F107xC
STM32F4_DEVICES = STM32F407xx STM32F469xx STM32F427xx

DEVICES = $(STM32F1_DEVICES) $(STM32F4_DEVICES)

## supported device FAMILIES
FAMILIES = STM32F1 STM32F4
FAMILY_FLAGS = -D STM32F1=1 -D STM32F4=4

ifeq ($(filter $(DEVICE),$(STM32F1_DEVICES)), $(DEVICE))
FAMILY = STM32F1
endif

ifeq ($(filter $(DEVICE),$(STM32F4_DEVICES)), $(DEVICE))
FAMILY = STM32F4
endif

## test FAMILY againt FAMILIES
ifeq ($(filter $(FAMILY),$(FAMILIES)), )
$(error FAMILY '$(FAMILY)' not supported. supported FAMILIES: $(FAMILIES))
endif

## common memory setup
RAM_BASE_ADDRESS=0x20000000
FLASH_BASE_ADDRESS=0x08000000

## application base address offset
# default to run from FLASH_BASE_ADDRESS + APP_ADDRESS_OFFSET=0
ifeq ($(APP_ADDRESS_OFFSET), )
APP_ADDRESS_OFFSET=0x0000
endif

## Configure FAMILY
ifeq ($(FAMILY), STM32F1)
FREERTOS_PORT = GCCARMCM3
CPU_FLAGS = -mthumb -mcpu=cortex-m3
FPU_FLAGS =
CHIPSUPPORTDIR = $(DEVICEDIR)/CM3
CHIPSUPPORT_MK = stm32f10x.mk
# stm32f1 memory setup 
STACK_RAM = RAM
CCRAM_BASE_ADDRESS=0x00000000
HALPREFIX = STM32F1xx
HSI_VALUE = 8000000
endif

ifeq ($(FAMILY), STM32F4)
FREERTOS_PORT = GCCARMCM4
CPU_FLAGS = -mthumb -mcpu=cortex-m4
FPU_FLAGS += -D __FPU_PRESENT=1 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fno-strict-aliasing -Wdouble-promotion -fsingle-precision-constant
CHIPSUPPORT_MK = stm32f4xx.mk
CHIPSUPPORTDIR = $(DEVICEDIR)/CM4
# stm32f4 memory setup 
STACK_RAM = CCRAM
CCRAM_BASE_ADDRESS=0x10000000
HALPREFIX = STM32F4xx
HSI_VALUE = 16000000
endif

## test DEVICE against DEVICES
ifeq ($(filter $(DEVICE),$(DEVICES)), )
$(error device '$(DEVICE)' not supported. supported devices: $(DEVICES))
endif

####################
## configure DEVICE
####################

ifeq ($(DEVICE), STM32F103xE)
# device specification
INTERRUPT_HANDLER_SOURCE = interrupt_handlers_hd.c
RAM_LENGTH ?= 64K
CCRAM_LENGTH ?= 0K				# no ccram
FLASH_LENGTH ?= 512K
MIN_HEAP_SIZE ?= 0x3000
MIN_STACK_SIZE ?= 0x0400		# min 1kb stack
endif

ifeq ($(DEVICE), STM32F107xC)
# device specification
INTERRUPT_HANDLER_SOURCE = interrupt_handlers_cl.c
RAM_LENGTH ?= 64K
CCRAM_LENGTH ?= 0K				# no ccram
FLASH_LENGTH ?= 256K
MIN_HEAP_SIZE ?= 0x3000
MIN_STACK_SIZE ?= 0x0400		# min 1kb stack
endif

ifeq ($(DEVICE), STM32F407xx)
# device specification
INTERRUPT_HANDLER_SOURCE = interrupt_handlers.c
RAM_LENGTH ?= 128K
CCRAM_LENGTH ?= 64K
FLASH_LENGTH ?= 512K
MIN_HEAP_SIZE ?= 0x3000
MIN_STACK_SIZE ?= 0x10000		# 64kb stack in ccram
endif

# todo - make a method to specify the xx part (determines pin count and memory size variations)
ifeq ($(DEVICE), STM32F407xx)
# device specification
INTERRUPT_HANDLER_SOURCE = interrupt_handlers.c
RAM_LENGTH ?= 128K
CCRAM_LENGTH ?= 64K
FLASH_LENGTH ?= 1024K
MIN_HEAP_SIZE ?= 0x3000
MIN_STACK_SIZE ?= 0x10000		# 64kb stack in ccram
endif

ifeq ($(DEVICE), STM32F427xx)
# device specification
INTERRUPT_HANDLER_SOURCE = interrupt_handlers.c
RAM_LENGTH ?= 192K
CCRAM_LENGTH ?= 64K
FLASH_LENGTH ?= 2048K
MIN_HEAP_SIZE ?= 0x3000
MIN_STACK_SIZE ?= 0x10000		# 64kb stack in ccram
endif

ifeq ($(DEVICE), STM32F469xx)
# device specification
INTERRUPT_HANDLER_SOURCE = interrupt_handlers.c
RAM_LENGTH ?= 256K
CCRAM_LENGTH ?= 64K
FLASH_LENGTH ?= 2048K
MIN_HEAP_SIZE ?= 0x3000
MIN_STACK_SIZE ?= 0x10000		# 64kb stack in ccram
endif

## TODO - this linker script is good for c++ - can it be make simpler for c only projects?
BASE_LINKER_SCRIPT = atollic_arm_cpp.ld
LDSCRIPT = stm32.ld
LINKERSCRIPTPATH = $(DEVICEDIR)

#STARTUP_SOURCE = $(DEVICEDIR)/startup.c
STARTUP_SOURCE += $(CHIPSUPPORTDIR)/$(INTERRUPT_HANDLER_SOURCE)

## configure  CFLAGS
# linker script
CFLAGS += -T$(LDSCRIPT)
#linker script paths
CFLAGS += -L$(LINKERSCRIPTPATH)
CFLAGS += $(FAMILY_FLAGS)
CFLAGS += $(CPU_FLAGS) $(FPU_FLAGS)
CFLAGS += -D $(FREERTOS_PORT)
CFLAGS += -D $(DEVICE)
CFLAGS += -I $(CHIPSUPPORTDIR)/cube_drivers/Inc
CFLAGS += -I $(CHIPSUPPORTDIR)/cube_drivers/CMSIS/Include
CFLAGS += -I $(CHIPSUPPORTDIR)/cube_drivers/CMSIS/Device/ST/$(HALPREFIX)/Include
CFLAGS += -I $(CHIPSUPPORTDIR)/cube_drivers/$(HALPREFIX)_HAL_Driver/Inc
CFLAGS += -I $(DEVICEDIR)
CFLAGS += -I $(CHIPSUPPORTDIR)

include $(CHIPSUPPORTDIR)/$(CHIPSUPPORT_MK)

buildlinkerscript:
	@echo 
	@echo "building linker script"
	@cp $(LINKERSCRIPTPATH)/$(LDSCRIPT).in $(LINKERSCRIPTPATH)/$(LDSCRIPT)
	@sed -i 's/!RAM_LENGTH!/$(RAM_LENGTH)/g' $(LINKERSCRIPTPATH)/$(LDSCRIPT)
	@sed -i 's/!RAM_BASE_ADDRESS!/$(RAM_BASE_ADDRESS)/g' $(LINKERSCRIPTPATH)/$(LDSCRIPT)
	@sed -i 's/!FLASH_BASE_ADDRESS!/$(FLASH_BASE_ADDRESS)/g' $(LINKERSCRIPTPATH)/$(LDSCRIPT)
	@sed -i 's/!FLASH_LENGTH!/$(FLASH_LENGTH)/g' $(LINKERSCRIPTPATH)/$(LDSCRIPT)
	@sed -i 's/!CCRAM_BASE_ADDRESS!/$(CCRAM_BASE_ADDRESS)/g' $(LINKERSCRIPTPATH)/$(LDSCRIPT)
	@sed -i 's/!CCRAM_LENGTH!/$(CCRAM_LENGTH)/g' $(LINKERSCRIPTPATH)/$(LDSCRIPT)
	@sed -i 's/!APP_ADDRESS_OFFSET!/$(APP_ADDRESS_OFFSET)/g' $(LINKERSCRIPTPATH)/$(LDSCRIPT)
	@sed -i 's/!MIN_STACK_SIZE!/$(MIN_STACK_SIZE)/g' $(LINKERSCRIPTPATH)/$(LDSCRIPT)
	@sed -i 's/!MIN_HEAP_SIZE!/$(MIN_HEAP_SIZE)/g' $(LINKERSCRIPTPATH)/$(LDSCRIPT)
	@sed -i 's/!STACK_RAM!/$(STACK_RAM)/g' $(LINKERSCRIPTPATH)/$(LDSCRIPT)
	@sed -i 's/!BASE_LINKER_SCRIPT!/$(BASE_LINKER_SCRIPT)/g' $(LINKERSCRIPTPATH)/$(LDSCRIPT)
	
