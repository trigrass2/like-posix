

CFLAGS += -DUSE_WAV_AUDIO=$(USE_WAV_AUDIO)
CFLAGS += -DUSE_WAV_STREAM=$(USE_WAV_STREAM)
CFLAGS += -DUSE_SIGGEN_STREAM=$(USE_SIGGEN_STREAM)

#####################
# USE_WAV_AUDIO
#####################
ifeq ($(USE_WAV_AUDIO), 1)

ifeq ($(USE_POSIX_STYLE_IO), 0)
$(error USE_WAV_AUDIO is set. wave audio utils require USE_POSIX_STYLE_IO set to 1)
endif

CFLAGS += -I $(AUTENSILS_DIR)/wav
SOURCE += $(AUTENSILS_DIR)/wav/wav.c
endif


#####################
# USE_WAV_STREAM
#####################

ifeq ($(USE_WAV_STREAM), 1)

ifeq ($(USE_DRIVER_I2S_STREAM), 0)
$(error USE_WAV_STREAM is set. wave stream utils require USE_DRIVER_I2S_STREAM set to 1)
endif

ifeq ($(USE_POSIX_STYLE_IO), 0)
$(error USE_WAV_STREAM is set. wave stream utils require USE_POSIX_STYLE_IO set to 1)
endif

CFLAGS += -I $(AUTENSILS_DIR)/wavstream
SOURCE += $(AUTENSILS_DIR)/wavstream/wavstream.c
endif

#####################
# USE_SIGGEN_STREAM
#####################
ifeq ($(USE_SIGGEN_STREAM), 1)

ifeq ($(USE_DRIVER_I2S_STREAM), 0)
$(error USE_SIGGEN_STREAM is set. signal generator stream utils require USE_DRIVER_I2S_STREAM set to 1)
endif

CFLAGS += -I $(AUTENSILS_DIR)/siggen
SOURCE += $(AUTENSILS_DIR)/siggen/siggen.c
SOURCE += $(AUTENSILS_DIR)/siggen/lut.c
endif
