///////////////////////////////////////////////////////////////////////////////
//Brad Goold 2012
// Modified by Mike Stuart, Feb 2014
///////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include "board_config.h"
#include "ds1820_config.h.in"

#ifndef DS1820_H
#define DS1820_H

#define DS1820_MAX_VALUE              125
#define DS1820_MIN_VALUE              -55

// ROM COMMANDS
#define MATCH_ROM           0x55
#define SEARCH_ROM          0xF0
#define SKIP_ROM            0xCC
#define READ_ROM            0x33
#define ALARM_SEARCH        0xEC

// FUNCTION COMMANDS
#define CONVERT_TEMP        0x44
#define COPY_SCRATCHPAD     0x48
#define WRITE_SCRATCHPAD    0x4E
#define READ_SCRATCHPAD     0xBE
#define RECALL_EEPROM       0xB8
#define READ_PS             0xB4

// error codes
#define BUS_ERROR           0xFE
#define PRESENCE_ERROR      0xFD
#define NO_ERROR            0x00

void ds1820_init(void);
uint64_t ds1820_search();
void ds1820_convert(void);
float ds1820_read_device(uint64_t devcode);

#endif
