minstdlibs
==========

tiny implementations of c standard library functions.

The code is targeted for use on small embedded targets where memeory is limited.

Min stdio
---------

the minimal standard io code has some dependancies - the functions _read, _write, _lseek, _ftell 
must be defined somewhere in your project.

they are typically defined in syscalls.c... just #include <stdio.h> like normal wherever you want to use stdio.

````
// examples of complete implementations targeting FreeRTOS and FatFs by ChaN may be found in syscalls.c in
// the stm32_freertos project, https://github.com/drmetal/stm32_freertos

// example of _write, function to write a characters to a device or file
int _write(int file, char *buffer, unsigned int count)
{
	int n = EOF;

	if(file == STDOUT_FILENO || file == STDERR_FILENO)
	{
		for(n = 0; n < (int)count; n++) {
			phy_putc(*buffer++); // some code here to write a serial device, or memory, or whatever
		}
	}
	return n;
}
// example of _read, function to read a characters from a device or file
int _read(int file, char *buffer, unsigned int count)
{
	int n = EOF;

	if(file == STDOUT_FILENO || file == STDERR_FILENO)
	{
		for(n = 0; n < (int)count; n++) {
			phy_getc(*buffer++); // some code here to read a serial device, or memory, or whatever
		}
	}

	return n;
}
// example of _lseek, function to change the position of the file pointer
extern int _lseek(int file, int offset, int whence)
{
	return -1;
}
// example of _ftell, function to read the position of the file pointer
extern long int _ftell(int fd)
{
	return -1;
}

````

supported string formatting:

 - %%		print % symbol
 - %c		print character
 - %s		print null terminated string
 - %i		print signed int
 - %d		print signed int
 - %u		print unsigned int
 - %x		print unsigned int, in hexadecimal format, lower case
 - %X		print unsigned int, in hexadecimal format, upper case
 - %p		print pointer address, in hexadecimal format, lower case, with '0x' presceeding
 - %f		print floating point value
 - 0, space	padding with space and 0 characters are supported by i, u, d, x, X, p
			Eg printf("padded integer: %06d", 123);
				-> padded integer: 000123
			Eg printf("padded integer: % 6d\n", 123);
			   printf("padded integer: % 6d", 4567);
				-> padded integer:    123
				-> padded integer:   4567
 - #     	a '0x' preceeds hexadecimal formatted numbers when # is inserted. supported by x, X:
			Eg printf("modified hex: %#x", 1234);
			   printf("modified hex: %#X", 1234);
				-> modified hex: 0x4d2
				-> modified hex: 0x4D2
 - l,h		ignored.
			Eg printf("ignore lh: %llu %lld %lu %ld %hu %hd", 1234, 1234, 1234, 1234, 1234, 1234);
				-> ignore lh: 1234 1234 1234 1234 1234 1234
 - +		a + preceeds numeric formatted numbers when + is inserted. supported by i, u, d, f:



Testing
-------

Setting up gtest

```
# acquire, build and install gtest
wget http://googletest.googlecode.com/files/gtest-1.7.0.zip
unzip gtest-1.7.0.zip
cd gtest-1.7.0
./configure
make
sudo cp -a include/gtest /usr/include
sudo cp -a lib/.libs/* /usr/lib/
sudo ldconfig -v | grep gtest
# check that the gtest so's are in place
libgtest.so.0 -> libgtest.so.0.0.0
libgtest_main.so.0 -> libgtest_main.so.0.0.0
# optionally remove gtest sources
cd ..
rm -rf gtest-1.7.0

```

Building and runnning the tests

```
cd minstdlibs/test
make clean all run
```
