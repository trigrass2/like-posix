
USE_GRAPHICS_VALUES = 0 1

ifeq ($(filter $(USE_GRAPHICS),$(USE_GRAPHICS_VALUES)), )
$(error USE_GRAPHICS is not set. set to one of: $(USE_GRAPHICS_VALUES))
endif

CFLAGS += -DUSE_GRAPHICS=$(USE_GRAPHICS)

ifeq ($(USE_GRAPHICS), 1)

CFLAGS += -I $(GRAPHICSDIR)
CFLAGS += -I $(GRAPHICSDIR)/fonts
CFLAGS += -I $(GRAPHICSDIR)/images
CFLAGS += -I $(GRAPHICSDIR)/widgets

SOURCE += $(GRAPHICSDIR)/graphics.c
SOURCE += $(GRAPHICSDIR)/text.c
SOURCE += $(GRAPHICSDIR)/image.c
SOURCE += $(GRAPHICSDIR)/shape.c

SOURCE += $(GRAPHICSDIR)/images/images.c

SOURCE += $(GRAPHICSDIR)/fonts/FontAwesome_16.c
SOURCE += $(GRAPHICSDIR)/fonts/FontAwesome_20.c
SOURCE += $(GRAPHICSDIR)/fonts/FontAwesome_24.c
SOURCE += $(GRAPHICSDIR)/fonts/FontAwesome_32.c
SOURCE += $(GRAPHICSDIR)/fonts/FontAwesome_48.c
SOURCE += $(GRAPHICSDIR)/fonts/FontAwesome_64.c
SOURCE += $(GRAPHICSDIR)/fonts/FontAwesome_16_Inverted.c
SOURCE += $(GRAPHICSDIR)/fonts/FontAwesome_20_Inverted.c
SOURCE += $(GRAPHICSDIR)/fonts/FontAwesome_24_Inverted.c
SOURCE += $(GRAPHICSDIR)/fonts/FontAwesome_32_Inverted.c
SOURCE += $(GRAPHICSDIR)/fonts/FontAwesome_48_Inverted.c
SOURCE += $(GRAPHICSDIR)/fonts/FontAwesome_64_Inverted.c
SOURCE += $(GRAPHICSDIR)/fonts/Ubuntu_16.c
SOURCE += $(GRAPHICSDIR)/fonts/Ubuntu_20.c
SOURCE += $(GRAPHICSDIR)/fonts/Ubuntu_24.c
SOURCE += $(GRAPHICSDIR)/fonts/Ubuntu_32.c
SOURCE += $(GRAPHICSDIR)/fonts/Ubuntu_38.c
SOURCE += $(GRAPHICSDIR)/fonts/Ubuntu_48.c
SOURCE += $(GRAPHICSDIR)/fonts/Ubuntu_64.c
SOURCE += $(GRAPHICSDIR)/fonts/Ubuntu_48_bold.c
SOURCE += $(GRAPHICSDIR)/fonts/Ubuntu_64_bold.c
SOURCE += $(GRAPHICSDIR)/fonts/Digital_7_Italic_32.c
SOURCE += $(GRAPHICSDIR)/fonts/Digital_7_Italic_64.c
SOURCE += $(GRAPHICSDIR)/fonts/Digital_7_Italic_96.c
SOURCE += $(GRAPHICSDIR)/fonts/Digital_7_Italic_128.c

ifeq ($(USE_GRAPHIC_WIDGET_TOUCH_CAL), 1)
SOURCE += $(GRAPHICSDIR)/widgets/touch_cal.c
endif
ifeq ($(USE_GRAPHIC_WIDGET_TOUCH_KEY), 1)
SOURCE += $(GRAPHICSDIR)/widgets/touch_key.c
endif
ifeq ($(USE_GRAPHIC_WIDGET_PANEL_METER), 1)
SOURCE += $(GRAPHICSDIR)/widgets/panel_meter.c
endif
ifeq ($(USE_GRAPHIC_WIDGET_STATUSBAR), 1)
SOURCE += $(GRAPHICSDIR)/widgets/statusbar.c
endif
ifeq ($(USE_GRAPHIC_WIDGET_SLIDER), 1)
SOURCE += $(GRAPHICSDIR)/widgets/slider.c
endif
endif
