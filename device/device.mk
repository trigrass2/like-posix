
DEVICEDIR = $(DEVICE_SUPPORT_DIR)/device

## supported device FAMILIES
FAMILIES = STM32F1 STM32F4
FAMILY_FLAGS = -D STM32F1=1 -D STM32F4=4
## supported DEVICES
DEVICES = stm32f103ve stm32f107rc stm32f407ve stm32f407vg

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
endif

## test DEVICE against DEVICES
ifeq ($(filter $(DEVICE),$(DEVICES)), )
$(error device '$(DEVICE)' not supported. supported devices: $(DEVICES))
endif

####################
## configure DEVICE
####################

ifeq ($(DEVICE), stm32f103ve)
# device specification
INTERRUPT_HANDLER_SOURCE = interrupt_handlers_hd.c
DENSITY = STM32F10X_HD
# stm32f103ve memory setup 
RAM_LENGTH ?= 64K
CCRAM_LENGTH ?= 0K				# no ccram
FLASH_LENGTH ?= 512K
MIN_HEAP_SIZE ?= 0x3000
MIN_STACK_SIZE ?= 0x0400
endif

ifeq ($(DEVICE), stm32f107rc)
# device specification
INTERRUPT_HANDLER_SOURCE = interrupt_handlers_cl.c
DENSITY = STM32F10X_CL
# stm32f107rc memory setup 
RAM_LENGTH ?= 64K
CCRAM_LENGTH ?= 0K				# no ccram
FLASH_LENGTH ?= 256K
MIN_HEAP_SIZE ?= 0x3000
MIN_STACK_SIZE ?= 0x0400
endif

ifeq ($(DEVICE), stm32f407ve)
# device specification
INTERRUPT_HANDLER_SOURCE = interrupt_handlers.c
DENSITY = STM32F4XX
# stm32f407ve memory setup 
RAM_LENGTH ?= 128K
CCRAM_LENGTH ?= 64K
FLASH_LENGTH ?= 512K
MIN_HEAP_SIZE ?= 0x3000
MIN_STACK_SIZE ?= 0x0000			# stack in ccram
endif

ifeq ($(DEVICE), stm32f407vg)
# device specification
INTERRUPT_HANDLER_SOURCE = interrupt_handlers.c
DENSITY = STM32F4XX
# stm32f407vg memory setup 
RAM_LENGTH ?= 128K
CCRAM_LENGTH ?= 64K
FLASH_LENGTH ?= 1024K
MIN_HEAP_SIZE ?= 0x3000
MIN_STACK_SIZE ?= 0x0000			# stack in ccram
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
CFLAGS += -D $(DENSITY)
CFLAGS += -I $(CHIPSUPPORTDIR)/std_periph_drivers/inc
CFLAGS += -I $(CHIPSUPPORTDIR)/ethernet
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
	
