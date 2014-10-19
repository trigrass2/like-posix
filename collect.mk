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


## project name
ifeq ($(PROJECT_NAME), )
PROJECT_NAME = main
endif

## Output Directory.
ifeq ($(OUTDIR), )
OUTDIR = bin
endif

OUTPUT_PREFIX = $(OUTDIR)/$(PROJECT_NAME)

## Toolchain definition.
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size
OBJDUMP=arm-none-eabi-objdump
NM = arm-none-eabi-nm

## C Compiler flags.
CFLAGS =

## include sub makefiles
include $(BUILD_ENV_DIR)/stm32_device_support.mk
include $(BUILD_ENV_DIR)/cutensils.mk
include $(BUILD_ENV_DIR)/minstdlibs.mk
include $(BUILD_ENV_DIR)/freertos.mk
include $(BUILD_ENV_DIR)/lwip.mk
include $(BUILD_ENV_DIR)/like_posix.mk
include $(BUILD_ENV_DIR)/fatfs.mk
include $(BUILD_ENV_DIR)/programming.mk

## definitions
CFLAGS += -D FAMILY=$(FAMILY)
CFLAGS += -D DEVICE=$(DEVICE)
CFLAGS += -D HSE_VALUE=$(HSE_VALUE)
CFLAGS += -D USE_FULL_ASSERT
CFLAGS += -D USE_STDPERIPH_DRIVER
CFLAGS += -D inline=

## compile options
ifneq ($(DEBUG), )
CFLAGS += -g$(DEBUG)
endif
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

## Linker options
# to allow for C++, removed -nostartfiles
LINKER_FLAGS = -Xlinker -o$(OUTPUT_PREFIX).elf -Xlinker -M -Xlinker -Map=$(OUTPUT_PREFIX).map -Xlinker
## note, if debug is enabled use -no-gc-sections - binaries will be unoptimised, read enormous
LINKER_FLAGS +=  -gc-sections
#ifeq ($(DEBUG), )
#LINKER_FLAGS +=  -gc-sections
#else
#LINKER_FLAGS +=  -no-gc-sections
#endif

## add any libraries here
LIBS= -lm


####################################################################
#  Rules definition
####################################################################

# List of all source files without directory and file-extension.
ALLSRCBASE = $(notdir $(basename $(SOURCE)))
# List of all objects files.
OBJS = $(addprefix $(OUTDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

all: begin gccversion buildlinkerscript $(OUTPUT_PREFIX).bin log size end

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

# compile startup code
$(OUTDIR)/startup.o : $(STARTUP_SOURCE) Makefile
	$(CC) -c $(CFLAGS) -O1 $(STARTUP_SOURCE) -o $(OUTDIR)/startup.o

clean :
	-rm -f $(OBJS)
	-rm -f $(OUTDIR)/startup.o
	-rm -f $(OUTPUT_PREFIX).elf
	-rm -f $(OUTPUT_PREFIX).bin
	-rm -f $(OUTPUT_PREFIX).map
	-rm -f $(PROJECT_NAME)_SymbolTable.txt
	-rm -f $(PROJECT_NAME)_MemoryListingSummary.txt
	rm -f $(PROJECT_NAME)_MemoryListingDetails.txt

log : $(OUTPUT_PREFIX).elf
	$(NM) -n $(OUTPUT_PREFIX).elf > $(OUTPUT_PREFIX)_SymbolTable.txt
	$(SIZE) --format=SysV $(OUTPUT_PREFIX).elf > $(OUTPUT_PREFIX)_MemoryListingSummary.txt
	$(SIZE) $(OBJS) > $(OUTPUT_PREFIX)_MemoryListingDetails.txt


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

