
### Disk Drive API

Multi volume disk drive API.

#### Using the disk API

To use the fat filesystem with the dsik drive interface, add the following settings in the project makefile:

``` bash
# USE_DRIVER_FAT_FILESYSTEM is required
USE_DRIVER_FAT_FILESYSTEM = 1
# to use ramdisk(s)
USE_DRIVER_RAMDISK = 1
# to use SD card disk(s) (supporting SDIO and SPI)
USE_DRIVER_SDCARD = 1
# if using an SPI connected sdcard, set USE_DRIVER_SPI=1. to use SDIO connected sdcard, set USE_DRIVER_SPI=0.
USE_DRIVER_SPI = 1
```

* use the disk mount API's in xxxx_diskio.h for disk mount operations.
* use the generic disk drive API in diskdrive.h for generic disk operations.

#### To initialize a ramdisk

``` C
// create a ram, buffer to mount as a ramdisk - the minimum size is 128 sectors of 512 bytes each.
#include "ram_diskio.h"
char mem[RAMDISK_SS * RAMDISK_MIN_SECTOR_COUNT];
ramdisk_t rdisk;
disk_interface_t rdisk_drive;

ramdisk_mount(&rdisk_drive, 0, &rdisk, mem, sizeof(mem));
```

#### To initialize an SD card

``` C
#include "sdcard_diskio.h"

disk_interface_t sddisk;

sdcard_mount(&sddisk, 0);
```

#### To use multiple disks

``` C
#include "diskdrive.h"
#include "ram_diskio.h"
#include "sdcard_diskio.h"

#define RAMDISK_DISKNUM 0
#define SDCARD_DISKNUM 1

// increment the drive number for each mount
ramdisk_mount(&rdisk_drive, RAMDISK_DISKNUM, &rdisk, mem, sizeof(mem));
sdcard_mount(&sddisk, SDCARD_DISKNUM);

// select drive to use at run time 
diskdrive_chdrive(SDCARD_DISKNUM);
```

### File IO API's

there are two ways to to disk and file IO,

 - raw FatFs, use the file IO functions from **ff.h**
 - Posix style, in the project makefile set USE_LIKEPOSIX=1, use the file IO functions from **stdio.h, fcntl.h, unistd.h, dirent.h**, among others.
 



