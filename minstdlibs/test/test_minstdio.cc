
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "gtest/gtest.h"
#include "strutils.h"
#include "minstdio.h"

/**
 * test fixture for functions calling _write
 */
#define BUFFER_SIZE				1024
#define TEST_FILE_DESCRIPTOR 	((FILE*)10)

char buffer[BUFFER_SIZE];
uint32_t i = 0;

int _write(int file, char *buf, unsigned int count)
{
	int n = EOF;

	if(file == STDOUT_FILENO || file == STDERR_FILENO)
	{
		for(n = 0; n < (int)count; n++, i++) {
			buffer[i] = *buf;
			buf++;
		}
	}
	else if(file == (intptr_t)TEST_FILE_DESCRIPTOR)
	{
		for(n = 0; n < (int)count; n++, i++) {
			buffer[i] = *buf;
			buf++;
		}
	}
	return n;
}

int _read(int file, char *buf, unsigned int count)
{
	int n = EOF;

	if(file == STDOUT_FILENO || file == STDERR_FILENO)
	{
		for(n = 0; n < (int)count; n++, i++) {
			*buf = buffer[i];
			buf++;
		}
	}
	else if(file == (intptr_t)TEST_FILE_DESCRIPTOR)
	{
		for(n = 0; n < (int)count; n++, i++) {
			*buf = buffer[i];
			buf++;
		}
	}

	return n;
}

extern long int _ftell(int file)
{
	int n = EOF;

	if(file == STDOUT_FILENO || file == STDERR_FILENO)
	{
		n = i;
	}
	else if(file == (intptr_t)TEST_FILE_DESCRIPTOR)
	{
		n = i;
	}

	return n;
}

extern int _lseek(int file, int offset, int whence)
{
	int n = EOF;

	if(file == STDOUT_FILENO || file == STDERR_FILENO)
	{

	}
	else if(file == (intptr_t)TEST_FILE_DESCRIPTOR)
	{
		if(whence == SEEK_CUR)
			offset = _ftell(file) + offset;
		else if(whence == SEEK_END)
			offset = sizeof(buffer) - offset;

		i = offset;
		n = 0;
	}

	return n;
}


void reset_fixture()
{
	memset(buffer, 0, sizeof(buffer));
	i = 0;
}

char* get_buffer()
{
	return buffer;
}

/**
 * printf
 */

TEST(test_printf, test_printf_percent_d)
{
	int ret;

	reset_fixture();
	ret = printf("hello %d", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_printf, test_printf_percent_d_with_positive)
{
	int ret;

	reset_fixture();
	ret = printf("hello %+d", 12345);
	ASSERT_STREQ("hello +12345", get_buffer());
	ASSERT_EQ(ret, 12);
}

TEST(test_printf, test_printf_percent_d_negative_with_positive)
{
	int ret;

	reset_fixture();
	ret = printf("hello %+d", -12345);
	ASSERT_STREQ("hello -12345", get_buffer());
	ASSERT_EQ(ret, 12);
}

TEST(test_printf, test_printf_percent_d_negative)
{
	int ret;

	reset_fixture();
	ret = printf("hello %d", -12345);
	ASSERT_STREQ("hello -12345", get_buffer());
	ASSERT_EQ(ret, 12);
}

TEST(test_printf, test_printf_percent_d_overflow)
{
	int ret;

	reset_fixture();
	ret = printf("hello %d", (int)3000000000UL);
	ASSERT_STREQ("hello -1294967296", get_buffer());
	ASSERT_EQ(ret, 17);
}

TEST(test_printf, test_printf_percent_i)
{
	int ret;

	reset_fixture();
	ret = printf("hello %i", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_printf, test_printf_percent_i_negative)
{
	int ret;

	reset_fixture();
	ret = printf("hello %i", -12345);
	ASSERT_STREQ("hello -12345", get_buffer());
	ASSERT_EQ(ret, 12);
}

TEST(test_printf, test_printf_percent_i_overflow)
{
	int ret;

	reset_fixture();
	ret = printf("hello %i", (int)3000000000UL);
	ASSERT_STREQ("hello -1294967296", get_buffer());
	ASSERT_EQ(ret, 17);
}

TEST(test_printf, test_printf_percent_u)
{
	int ret;

	reset_fixture();
	ret = printf("hello %u", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_printf, test_printf_percent_u_no_overflow)
{
	int ret;

	reset_fixture();
	ret = printf("hello %u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 3000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_u_overflow)
{
	int ret;

	reset_fixture();
	ret = printf("hello %u", -12345);
	ASSERT_STREQ("hello 4294954951", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_x)
{
	int ret;

	reset_fixture();
	ret = printf("hello %x", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_printf, test_printf_percent_x_no_overflow)
{
	int ret;

	reset_fixture();
	ret = printf("hello %x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello b2d05e00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_printf, test_printf_percent_X)
{
	int ret;

	reset_fixture();
	ret = printf("hello %X", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_printf, test_printf_percent_X_no_overflow)
{
	int ret;

	reset_fixture();
	ret = printf("hello %X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello B2D05E00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_printf, test_printf_percent_p)
{
	int ret;

	reset_fixture();
	ret = printf("hello %p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x3ade68b1", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_s)
{
	int ret;

	reset_fixture();
	ret = printf("hello %s", (char*)"whatsup");
	ASSERT_STREQ("hello whatsup", get_buffer());
	ASSERT_EQ(ret, 13);
}

TEST(test_printf, test_printf_percent_c)
{
	int ret;

	reset_fixture();
	ret = printf("hello %c", 'x');
	ASSERT_STREQ("hello x", get_buffer());
	ASSERT_EQ(ret, 7);
}

TEST(test_printf, test_printf_percent_d_with_0_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %012d", 12345);
	ASSERT_STREQ("hello 000000012345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_d_with_0_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello %08d", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_i_with_0_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %012i", 12345);
	ASSERT_STREQ("hello 000000012345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_i_with_0_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello %08i", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_u_with_0_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %012u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 003000000000", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_u_with_0_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello %08u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 3000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_x_with_0_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %012x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0000b2d05e00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_x_with_0_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello %08x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello b2d05e00", get_buffer());
	ASSERT_EQ(ret, 14);
}


TEST(test_printf, test_printf_percent_X_with_0_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %012X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0000B2D05E00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_X_with_0_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello %08X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello B2D05E00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_printf, test_printf_percent_p_with_0_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %012p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x00003ade68b1", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_printf, test_printf_percent_p_with_0_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello %08p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x3ade68b1", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_x_with_hash)
{
	int ret;

	reset_fixture();
	ret = printf("hello %#x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0xb2d05e00", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_x_with_hash_and_0_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %#012x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x0000b2d05e00", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_printf, test_printf_percent_X_with_hash_and_0_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %#012X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x0000B2D05E00", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_printf, test_printf_percent_X_with_hash)
{
	int ret;

	reset_fixture();
	ret = printf("hello %#X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0xB2D05E00", get_buffer());
	ASSERT_EQ(ret, 16);
}


TEST(test_printf, test_printf_percent_d_with_space_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello % 12d", 12345);
	ASSERT_STREQ("hello        12345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_d_with_space_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello % 8d", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_i_with_space_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello % 12i", 12345);
	ASSERT_STREQ("hello        12345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_i_with_space_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello % 8i", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_u_with_space_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello % 12u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello   3000000000", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_u_with_space_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello % 8u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 3000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_x_with_space_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello % 12x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello     b2d05e00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_x_with_space_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello % 8x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello b2d05e00", get_buffer());
	ASSERT_EQ(ret, 14);
}


TEST(test_printf, test_printf_percent_X_with_space_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello % 12X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello     B2D05E00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_X_with_space_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello % 8X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello B2D05E00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_printf, test_printf_percent_p_with_space_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello % 12p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x    3ade68b1", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_printf, test_printf_percent_p_with_space_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello % 8p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x3ade68b1", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_x_with_hash_and_space_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %# 12x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x    b2d05e00", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_printf, test_printf_percent_X_with_hash_and_space_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %# 12X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x    B2D05E00", get_buffer());
	ASSERT_EQ(ret, 20);
}

////////

TEST(test_printf, test_printf_percent_d_with_empty_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %12d", 12345);
	ASSERT_STREQ("hello        12345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_d_with_empty_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello %8d", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_i_with_empty_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %12i", 12345);
	ASSERT_STREQ("hello        12345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_i_with_empty_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello %8i", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_u_with_empty_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %12u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello   3000000000", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_u_with_empty_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello %8u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 3000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_x_with_empty_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %12x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello     b2d05e00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_x_with_empty_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello %8x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello b2d05e00", get_buffer());
	ASSERT_EQ(ret, 14);
}


TEST(test_printf, test_printf_percent_X_with_empty_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %12X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello     B2D05E00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_printf, test_printf_percent_X_with_empty_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello %8X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello B2D05E00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_printf, test_printf_percent_p_with_empty_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %12p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x    3ade68b1", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_printf, test_printf_percent_p_with_empty_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = printf("hello %8p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x3ade68b1", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_printf, test_printf_percent_x_with_hash_and_empty_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %#12x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x    b2d05e00", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_printf, test_printf_percent_X_with_hash_and_empty_padding)
{
	int ret;

	reset_fixture();
	ret = printf("hello %#12X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x    B2D05E00", get_buffer());
	ASSERT_EQ(ret, 20);
}
///////

TEST(test_printf, test_printf_percent_percent)
{	int ret;

	reset_fixture();
	ret = printf("hello %% %%\n");
	ASSERT_STREQ("hello % %\n", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_printf, test_printf_percent_ld)
{
	int ret;

	reset_fixture();
	ret = printf("hello %ld", (long int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_printf, test_printf_percent_lld)
{
	int ret;

	reset_fixture();
	ret = printf("hello %lld", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_printf, test_printf_percent_hd)
{
	int ret;

	reset_fixture();
	ret = printf("hello %hd", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_printf, test_printf_percent_hhd)
{
	int ret;

	reset_fixture();
	ret = printf("hello %hhd", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}


TEST(test_printf, test_printf_percent_li)
{
	int ret;

	reset_fixture();
	ret = printf("hello %li", (long int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_printf, test_printf_percent_lli)
{
	int ret;

	reset_fixture();
	ret = printf("hello %lli", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_printf, test_printf_percent_hi)
{
	int ret;

	reset_fixture();
	ret = printf("hello %hi", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_printf, test_printf_percent_hhi)
{
	int ret;

	reset_fixture();
	ret = printf("hello %hhi", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}


TEST(test_printf, test_printf_percent_lu)
{
	int ret;

	reset_fixture();
	ret = printf("hello %lu", (long unsigned int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_printf, test_printf_percent_llu)
{
	int ret;

	reset_fixture();
	ret = printf("hello %llu", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_printf, test_printf_percent_hu)
{
	int ret;

	reset_fixture();
	ret = printf("hello %hu", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_printf, test_printf_percent_hhu)
{
	int ret;

	reset_fixture();
	ret = printf("hello %hhu", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}


TEST(test_printf, test_printf_percent_lx)
{
	int ret;

	reset_fixture();
	ret = printf("hello %lx", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_printf, test_printf_percent_llx)
{
	int ret;

	reset_fixture();
	ret = printf("hello %llx", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_printf, test_printf_percent_hx)
{
	int ret;

	reset_fixture();
	ret = printf("hello %hx", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_printf, test_printf_percent_hhx)
{
	int ret;

	reset_fixture();
	ret = printf("hello %hhx", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}


/**
 * sprintf
 */

TEST(test_sprintf, test_sprintf_percent_d)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %d", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, test_sprintf_percent_d_with_positive)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %+d", 12345);
	ASSERT_STREQ("hello +12345", get_buffer());
	ASSERT_EQ(ret, 12);
}

TEST(test_sprintf, test_sprintf_percent_d_negative_with_positive)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %+d", -12345);
	ASSERT_STREQ("hello -12345", get_buffer());
	ASSERT_EQ(ret, 12);
}

TEST(test_sprintf, test_sprintf_percent_d_negative)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %d", -12345);
	ASSERT_STREQ("hello -12345", get_buffer());
	ASSERT_EQ(ret, 12);
}

TEST(test_sprintf, test_sprintf_percent_d_overflow)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %d", (int)3000000000UL);
	ASSERT_STREQ("hello -1294967296", get_buffer());
	ASSERT_EQ(ret, 17);
}

TEST(test_sprintf, test_sprintf_percent_i)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %i", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, test_sprintf_percent_i_negative)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %i", -12345);
	ASSERT_STREQ("hello -12345", get_buffer());
	ASSERT_EQ(ret, 12);
}

TEST(test_sprintf, test_sprintf_percent_i_overflow)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %i", (int)3000000000UL);
	ASSERT_STREQ("hello -1294967296", get_buffer());
	ASSERT_EQ(ret, 17);
}

TEST(test_sprintf, test_sprintf_percent_u)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %u", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, test_sprintf_percent_u_no_overflow)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 3000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_u_overflow)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %u", -12345);
	ASSERT_STREQ("hello 4294954951", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_x)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %x", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_sprintf, test_sprintf_percent_x_no_overflow)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello b2d05e00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_sprintf, test_sprintf_percent_X)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %X", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_sprintf, test_sprintf_percent_X_no_overflow)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello B2D05E00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_sprintf, test_sprintf_percent_p)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x3ade68b1", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_s)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %s", (char*)"whatsup");
	ASSERT_STREQ("hello whatsup", get_buffer());
	ASSERT_EQ(ret, 13);
}

TEST(test_sprintf, test_sprintf_percent_c)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %c", 'x');
	ASSERT_STREQ("hello x", get_buffer());
	ASSERT_EQ(ret, 7);
}

TEST(test_sprintf, test_sprintf_percent_d_with_0_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %012d", 12345);
	ASSERT_STREQ("hello 000000012345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_d_with_0_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %08d", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_i_with_0_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %012i", 12345);
	ASSERT_STREQ("hello 000000012345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_i_with_0_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %08i", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_u_with_0_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %012u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 003000000000", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_u_with_0_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %08u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 3000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_x_with_0_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %012x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0000b2d05e00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_x_with_0_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %08x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello b2d05e00", get_buffer());
	ASSERT_EQ(ret, 14);
}


TEST(test_sprintf, test_sprintf_percent_X_with_0_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %012X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0000B2D05E00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_X_with_0_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %08X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello B2D05E00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_sprintf, test_sprintf_percent_p_with_0_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %012p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x00003ade68b1", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, test_sprintf_percent_p_with_0_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %08p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x3ade68b1", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_x_with_hash)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %#x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0xb2d05e00", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_x_with_hash_and_0_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %#012x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x0000b2d05e00", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, test_sprintf_percent_X_with_hash_and_0_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %#012X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x0000B2D05E00", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, test_sprintf_percent_X_with_hash)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %#X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0xB2D05E00", get_buffer());
	ASSERT_EQ(ret, 16);
}


TEST(test_sprintf, test_sprintf_percent_d_with_space_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello % 12d", 12345);
	ASSERT_STREQ("hello        12345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_d_with_space_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello % 8d", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_i_with_space_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello % 12i", 12345);
	ASSERT_STREQ("hello        12345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_i_with_space_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello % 8i", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_u_with_space_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello % 12u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello   3000000000", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_u_with_space_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello % 8u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 3000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_x_with_space_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello % 12x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello     b2d05e00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_x_with_space_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello % 8x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello b2d05e00", get_buffer());
	ASSERT_EQ(ret, 14);
}


TEST(test_sprintf, test_sprintf_percent_X_with_space_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello % 12X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello     B2D05E00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_X_with_space_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello % 8X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello B2D05E00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_sprintf, test_sprintf_percent_p_with_space_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello % 12p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x    3ade68b1", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, test_sprintf_percent_p_with_space_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello % 8p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x3ade68b1", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_x_with_hash_and_space_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %# 12x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x    b2d05e00", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, test_sprintf_percent_X_with_hash_and_space_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %# 12X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x    B2D05E00", get_buffer());
	ASSERT_EQ(ret, 20);
}

///

TEST(test_sprintf, test_sprintf_percent_d_with_empty_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %12d", 12345);
	ASSERT_STREQ("hello        12345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_d_with_empty_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %8d", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_i_with_empty_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %12i", 12345);
	ASSERT_STREQ("hello        12345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_i_with_empty_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %8i", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_u_with_empty_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %12u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello   3000000000", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_u_with_empty_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %8u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 3000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_x_with_empty_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %12x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello     b2d05e00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_x_with_empty_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %8x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello b2d05e00", get_buffer());
	ASSERT_EQ(ret, 14);
}


TEST(test_sprintf, test_sprintf_percent_X_with_empty_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %12X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello     B2D05E00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, test_sprintf_percent_X_with_empty_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %8X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello B2D05E00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_sprintf, test_sprintf_percent_p_with_empty_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %12p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x    3ade68b1", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, test_sprintf_percent_p_with_empty_padding_undersized)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %8p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x3ade68b1", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, test_sprintf_percent_x_with_hash_and_empty_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %#12x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x    b2d05e00", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, test_sprintf_percent_X_with_hash_and_empty_padding)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %#12X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x    B2D05E00", get_buffer());
	ASSERT_EQ(ret, 20);
}
///
TEST(test_sprintf, test_sprintf_percent_percent)
{	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %% %%\n");
	ASSERT_STREQ("hello % %\n", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_sprintf, test_sprintf_percent_ld)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %ld", (long int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, test_sprintf_percent_lld)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %lld", (long long int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, test_sprintf_percent_hd)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %hd", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, test_sprintf_percent_hhd)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %hhd", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}


TEST(test_sprintf, test_sprintf_percent_li)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %li", (long int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, test_sprintf_percent_lli)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %lli", (long long int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, test_sprintf_percent_hi)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %hi", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, test_sprintf_percent_hhi)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %hhi", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}


TEST(test_sprintf, test_sprintf_percent_lu)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %lu", (long unsigned int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, test_sprintf_percent_llu)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %llu", (long long unsigned int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, test_sprintf_percent_hu)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %hu", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, test_sprintf_percent_hhu)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %hhu", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}


TEST(test_sprintf, test_sprintf_percent_lx)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %lx", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_sprintf, test_sprintf_percent_llx)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %llx", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_sprintf, test_sprintf_percent_hx)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %hx", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_sprintf, test_sprintf_percent_hhx)
{
	int ret;

	reset_fixture();
	ret = sprintf(get_buffer(), "hello %hhx", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}


/**
 * fprintf
 */

TEST(test_fprintf, test_fprintf_percent_d)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %d", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_fprintf, test_fprintf_percent_d_with_positive)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %+d", 12345);
	ASSERT_STREQ("hello +12345", get_buffer());
	ASSERT_EQ(ret, 12);
}

TEST(test_fprintf, test_fprintf_percent_d_negative_with_positive)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %+d", -12345);
	ASSERT_STREQ("hello -12345", get_buffer());
	ASSERT_EQ(ret, 12);
}

TEST(test_fprintf, test_fprintf_percent_d_negative)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %d", -12345);
	ASSERT_STREQ("hello -12345", get_buffer());
	ASSERT_EQ(ret, 12);
}

TEST(test_fprintf, test_fprintf_percent_d_overflow)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %d", (int)3000000000UL);
	ASSERT_STREQ("hello -1294967296", get_buffer());
	ASSERT_EQ(ret, 17);
}

TEST(test_fprintf, test_fprintf_percent_i)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %i", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_fprintf, test_fprintf_percent_i_negative)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %i", -12345);
	ASSERT_STREQ("hello -12345", get_buffer());
	ASSERT_EQ(ret, 12);
}

TEST(test_fprintf, test_fprintf_percent_i_overflow)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %i", (int)3000000000UL);
	ASSERT_STREQ("hello -1294967296", get_buffer());
	ASSERT_EQ(ret, 17);
}

TEST(test_fprintf, test_fprintf_percent_u)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %u", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_fprintf, test_fprintf_percent_u_no_overflow)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 3000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_u_overflow)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %u", -12345);
	ASSERT_STREQ("hello 4294954951", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_x)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %x", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_fprintf, test_fprintf_percent_x_no_overflow)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello b2d05e00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_fprintf, test_fprintf_percent_X)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %X", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_fprintf, test_fprintf_percent_X_no_overflow)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello B2D05E00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_fprintf, test_fprintf_percent_p)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x3ade68b1", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_s)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %s", (char*)"whatsup");
	ASSERT_STREQ("hello whatsup", get_buffer());
	ASSERT_EQ(ret, 13);
}

TEST(test_fprintf, test_fprintf_percent_c)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %c", 'x');
	ASSERT_STREQ("hello x", get_buffer());
	ASSERT_EQ(ret, 7);
}

TEST(test_fprintf, test_fprintf_percent_d_with_0_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %012d", 12345);
	ASSERT_STREQ("hello 000000012345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_d_with_0_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %08d", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_i_with_0_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %012i", 12345);
	ASSERT_STREQ("hello 000000012345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_i_with_0_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %08i", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_u_with_0_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %012u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 003000000000", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_u_with_0_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %08u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 3000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_x_with_0_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %012x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0000b2d05e00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_x_with_0_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %08x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello b2d05e00", get_buffer());
	ASSERT_EQ(ret, 14);
}


TEST(test_fprintf, test_fprintf_percent_X_with_0_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %012X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0000B2D05E00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_X_with_0_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %08X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello B2D05E00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_fprintf, test_fprintf_percent_p_with_0_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %012p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x00003ade68b1", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_fprintf, test_fprintf_percent_p_with_0_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %08p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x3ade68b1", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_x_with_hash)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %#x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0xb2d05e00", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_x_with_hash_and_0_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %#012x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x0000b2d05e00", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_fprintf, test_fprintf_percent_X_with_hash_and_0_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %#012X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x0000B2D05E00", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_fprintf, test_fprintf_percent_X_with_hash)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %#X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0xB2D05E00", get_buffer());
	ASSERT_EQ(ret, 16);
}


TEST(test_fprintf, test_fprintf_percent_d_with_space_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello % 12d", 12345);
	ASSERT_STREQ("hello        12345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_d_with_space_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello % 8d", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_i_with_space_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello % 12i", 12345);
	ASSERT_STREQ("hello        12345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_i_with_space_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello % 8i", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_u_with_space_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello % 12u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello   3000000000", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_u_with_space_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello % 8u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 3000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_x_with_space_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello % 12x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello     b2d05e00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_x_with_space_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello % 8x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello b2d05e00", get_buffer());
	ASSERT_EQ(ret, 14);
}


TEST(test_fprintf, test_fprintf_percent_X_with_space_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello % 12X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello     B2D05E00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_X_with_space_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello % 8X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello B2D05E00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_fprintf, test_fprintf_percent_p_with_space_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello % 12p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x    3ade68b1", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_fprintf, test_fprintf_percent_p_with_space_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello % 8p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x3ade68b1", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_x_with_hash_and_space_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %# 12x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x    b2d05e00", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_fprintf, test_fprintf_percent_X_with_hash_and_space_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %# 12X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x    B2D05E00", get_buffer());
	ASSERT_EQ(ret, 20);
}

///

TEST(test_fprintf, test_fprintf_percent_d_with_empty_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %12d", 12345);
	ASSERT_STREQ("hello        12345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_d_with_empty_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %8d", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_i_with_empty_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %12i", 12345);
	ASSERT_STREQ("hello        12345", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_i_with_empty_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %8i", 2000000000);
	ASSERT_STREQ("hello 2000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_u_with_empty_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %12u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello   3000000000", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_u_with_empty_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %8u", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 3000000000", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_x_with_empty_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %12x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello     b2d05e00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_x_with_empty_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %8x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello b2d05e00", get_buffer());
	ASSERT_EQ(ret, 14);
}


TEST(test_fprintf, test_fprintf_percent_X_with_empty_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %12X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello     B2D05E00", get_buffer());
	ASSERT_EQ(ret, 18);
}

TEST(test_fprintf, test_fprintf_percent_X_with_empty_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %8X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello B2D05E00", get_buffer());
	ASSERT_EQ(ret, 14);
}

TEST(test_fprintf, test_fprintf_percent_p_with_empty_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %12p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x    3ade68b1", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_fprintf, test_fprintf_percent_p_with_empty_padding_undersized)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %8p", (uintptr_t)987654321);
	ASSERT_STREQ("hello 0x3ade68b1", get_buffer());
	ASSERT_EQ(ret, 16);
}

TEST(test_fprintf, test_fprintf_percent_x_with_hash_and_empty_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %#12x", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x    b2d05e00", get_buffer());
	ASSERT_EQ(ret, 20);
}

TEST(test_fprintf, test_fprintf_percent_X_with_hash_and_empty_padding)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %#12X", (unsigned int)3000000000UL);
	ASSERT_STREQ("hello 0x    B2D05E00", get_buffer());
	ASSERT_EQ(ret, 20);
}
///

TEST(test_fprintf, test_fprintf_percent_percent)
{	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %% %%\n");
	ASSERT_STREQ("hello % %\n", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_fprintf, test_fprintf_percent_ld)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %ld", (long int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_fprintf, test_fprintf_percent_lld)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %lld", (long long int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_fprintf, test_fprintf_percent_hd)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %hd", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_fprintf, test_fprintf_percent_hhd)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %hhd", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}


TEST(test_fprintf, test_fprintf_percent_li)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %li", (long int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_fprintf, test_fprintf_percent_lli)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %lli", (long long int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_fprintf, test_fprintf_percent_hi)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %hi", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_fprintf, test_fprintf_percent_hhi)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %hhi", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}


TEST(test_fprintf, test_fprintf_percent_lu)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %lu", (long unsigned int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_fprintf, test_fprintf_percent_llu)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %llu", (long long unsigned int)12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_fprintf, test_fprintf_percent_hu)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %hu", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}

TEST(test_fprintf, test_fprintf_percent_hhu)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %hhu", 12345);
	ASSERT_STREQ("hello 12345", get_buffer());
	ASSERT_EQ(ret, 11);
}


TEST(test_fprintf, test_fprintf_percent_lx)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %lx", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_fprintf, test_fprintf_percent_llx)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %llx", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_fprintf, test_fprintf_percent_hx)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %hx", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

TEST(test_fprintf, test_fprintf_percent_hhx)
{
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fprintf(fd, "hello %hhx", 12345);
	ASSERT_STREQ("hello 3039", get_buffer());
	ASSERT_EQ(ret, 10);
}

/**
 * fputs
 */

TEST(test_fputs, test_fputs)
{	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fputs("hello 123", fd);
	ASSERT_STREQ("hello 123", get_buffer());
	ASSERT_EQ(ret, 9);
}

/**
 * fputc
 */

TEST(test_fputc, test_fputc)
{
	char expect = '5';
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ASSERT_EQ(get_buffer()[0], '\0');
	ret = fputc(expect, fd);
	ASSERT_EQ(get_buffer()[0], expect);
}

/**
 * fgets
 */

TEST(test_fgets, test_fgets_no_newline)
{
	char buf[100];
	const char* expect = "hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234";

	char* ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	ret = fgets(get_buffer(), BUFFER_SIZE, fd+1);

	ASSERT_EQ(ret, (char*)NULL);

	reset_fixture();
	strcpy(get_buffer(), expect);

	ret = fgets(buf, sizeof(buf), fd);

	ASSERT_EQ(ret, buf);
//	ASSERT_STREQ(expect, buf); // buf is truncated to 99 chars long
	ASSERT_EQ((int)strlen(buf), (int)(sizeof(buf)-1));
}

TEST(test_fgets, test_fgets_with_newline)
{
	char buf[100];
	const char* expect = "hello 123\nwerwerwerwerwer";

	char* ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	ret = fgets(get_buffer(), BUFFER_SIZE, fd+1);

	ASSERT_EQ(ret, (char*)NULL);

	reset_fixture();
	strcpy(get_buffer(), expect);

	ret = fgets(buf, sizeof(buf), fd);

	ASSERT_EQ(ret, buf);
	ASSERT_STREQ("hello 123\n", buf);
	ASSERT_EQ((int)strlen(buf), (int)strlen("hello 123\n"));
}

/**
 * fgetc
 */

TEST(test_fgetc, test_fgetc)
{
	char expect = '5';
	int ret;
	FILE* fd = TEST_FILE_DESCRIPTOR;

	reset_fixture();
	ret = fgetc(fd);
	ASSERT_EQ(get_buffer()[0], '\0');

	reset_fixture();
	get_buffer()[0] = expect;
	ret = fgetc(fd);
	ASSERT_EQ(expect, (char)ret);
}
