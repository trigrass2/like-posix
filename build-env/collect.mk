#******************************************************************************
# @file      Makefile
# @author    Stefano Oliveri (software@stf12.net)
# @version   V2.0
# @date      22/06/2009
# @copy
#
# THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING USERS
# WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
# TIME. AS A RESULT, STEFANO OLIVERI SHALL NOT BE HELD LIABLE FOR ANY
# DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
# FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
# CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
#
# <h2><center>&copy; COPYRIGHT 2009 Stefano Oliveri</center></h2>
#******************************************************************************


## NOTE: project name is set in setup.mk

## Output Directory.
OUTDIR ?= bin

OUTPUT_PREFIX = $(OUTDIR)/$(PROJECT_NAME)

## Toolchain definition.
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size
OBJDUMP=arm-none-eabi-objdump
NM = arm-none-eabi-nm

## create empty C Compiler flags if needed
CFLAGS ?=

# create empty source list if needed
SOURCE ?= 

## create empty library flags if needed
LIBS ?=

# create empty linker list if needed
LINKER_FLAGS ?= 

## include sub makefiles
include $(BUILD_ENV_DIR)/stm32_device_support.mk
include $(BUILD_ENV_DIR)/cutensils.mk
include $(BUILD_ENV_DIR)/nutensils.mk
include $(BUILD_ENV_DIR)/minstdlibs.mk
include $(BUILD_ENV_DIR)/freertos.mk
include $(BUILD_ENV_DIR)/lwip.mk
include $(BUILD_ENV_DIR)/like_posix.mk
include $(BUILD_ENV_DIR)/fatfs.mk
include $(BUILD_ENV_DIR)/programming.mk
include $(BUILD_ENV_DIR)/graphics.mk

## definitions
CFLAGS += -D CLEVER_DEFAULT_INTERRUPT_HANDLER=$(CLEVER_DEFAULT_INTERRUPT_HANDLER)
CFLAGS += -D PROJECT_VERSION='"$(PROJECT_VERSION)"'
CFLAGS += -D PROJECT_NAME='"$(PROJECT_NAME)"'
CFLAGS += -D BOARD='"$(BOARD)"'
CFLAGS += -D DEVICE='"$(DEVICE)"'
CFLAGS += -D FAMILY=$(FAMILY)
CFLAGS += -D HSE_VALUE=$(HSE_VALUE)
CFLAGS += -D USE_STDPERIPH_DRIVER
CFLAGS += -D inline=
ifeq ($(USE_FULL_ASSERT), 1)
CFLAGS += -D USE_FULL_ASSERT
endif

## compile options
CFLAGS += -O$(OPT)
CFLAGS += -std=gnu99
CFLAGS += -I ./
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += -Werror
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wpointer-arith
CFLAGS += -Wno-strict-aliasing
CFLAGS += -Wno-unused-function
CFLAGS += -Wformat=0
CFLAGS += -Wl,-Map,$(OUTPUT_PREFIX).map
CFLAGS += -Wno-attributes

AFLAGS  = $(CPU_FLAGS) -I. -x assembler-with-cpp
AFLAGS += -D__ASSEMBLY__
ASFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))
#ASFLAGS += -Wa,-g$(DEBUG)
#ASFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

## Linker options
# to allow for C++, remove -nostartfiles
ifndef __cplusplus
LINKER_FLAGS += -nostartfiles
endif
LINKER_FLAGS += --specs=nano.specs
LINKER_FLAGS += -Xlinker -o$(OUTPUT_PREFIX).elf -Xlinker -M -Xlinker -Map=$(OUTPUT_PREFIX).map -Xlinker
## note, if debug is enabled use -no-gc-sections - binaries will be unoptimised, read enormous
LINKER_FLAGS +=  -gc-sections

## add any libraries here
LIBS += -lm


####################################################################
#  Rules definition
####################################################################

# List of all source files without directory and file-extension.
ALLSRCBASE = $(notdir $(basename $(SOURCE))) $(notdir $(basename $(ASOURCE))) 
# List of all objects files.
OBJS = $(addprefix $(OUTDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

all: begin gccversion buildlinkerscript $(OUTPUT_PREFIX).bin log size proj_version end

# binary file
$(OUTPUT_PREFIX).bin : $(OUTPUT_PREFIX).elf Makefile
	$(OBJCOPY) $(OUTPUT_PREFIX).elf -O binary $(OUTPUT_PREFIX).bin

# elf file
$(OUTPUT_PREFIX).elf : $(OBJS) $(OUTDIR)/startup.o Makefile
	$(CC) $(CFLAGS) $(OBJS) $(OUTDIR)/startup.o $(LIBS) $(LINKER_FLAGS)

# Compile: create object files from C source files.
define COMPILE_C_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $$< "->" $$@
	$(CC) -c  $$(CFLAGS) $$< -o $$@
endef
$(foreach src, $(SOURCE), $(eval $(call COMPILE_C_TEMPLATE, $(src))))

# Assemble: create object files from assembler source files.
define ASSEMBLE_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $$< "->" $$@
	$(CC) -c $$(ASFLAGS) $$< -o $$@
endef
$(foreach src, $(ASOURCE), $(eval $(call ASSEMBLE_TEMPLATE, $(src))))

# compile startup code
$(OUTDIR)/startup.o : $(STARTUP_SOURCE) Makefile
	$(CC) -c $(CFLAGS) -O1 $(STARTUP_SOURCE) -o $(OUTDIR)/startup.o

clean :
	-rm -f $(OBJS)
	-rm -f $(OUTDIR)/startup.o
	-rm -f $(OUTPUT_PREFIX).elf
	-rm -f $(OUTPUT_PREFIX).bin
	-rm -f $(OUTPUT_PREFIX).map
	-rm -f $(OUTDIR)/$(PROJECT_NAME)_SymbolTable.txt
	-rm -f $(OUTDIR)/$(PROJECT_NAME)_MemoryListingSummary.txt
	-rm -f $(OUTDIR)/$(PROJECT_NAME)_MemoryListingDetails.txt
	-rm -f $(LINKERSCRIPTPATH)/$(LDSCRIPT)

log : $(OUTPUT_PREFIX).elf
	$(NM) -n $(OUTPUT_PREFIX).elf > $(OUTPUT_PREFIX)_SymbolTable.txt
	$(SIZE) --format=SysV $(OUTPUT_PREFIX).elf > $(OUTPUT_PREFIX)_MemoryListingSummary.txt
	$(SIZE) $(OBJS) > $(OUTPUT_PREFIX)_MemoryListingDetails.txt
	$(OBJDUMP) -S $(OUTPUT_PREFIX).elf > $(OUTPUT_PREFIX)_Disassembly.txt


begin:
	@echo -------- begin --------

end:
	@echo --------  end  --------

# Display compiler version information.
gccversion :
	@$(CC) --version

$(shell mkdir $(OUTDIR) 2>/dev/null)

ELFSIZE = $(SIZE) -B $(OUTPUT_PREFIX).elf
size:
	@if [ -f  $(OUTPUT_PREFIX).elf ]; then echo; echo "Size:"; $(ELFSIZE); echo; fi
	
proj_version:
	@echo Project Name: $(PROJECT_NAME)
	@echo Project Version: $(PROJECT_VERSION)
	@echo Date: $(DATE)
	@echo Board: $(BOARD)
