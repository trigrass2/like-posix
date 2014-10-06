
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "gtest/gtest.h"
#include "strutils.h"
#include "minstdio.h"

/**
 * test fixture for functions calling _write
 */
char buffer[1024];
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
 * stdio
 */

TEST(test_printf, test_printf_percent_d)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %d";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_d_with_positive)
{
	const char* expect = "hello +12345";
	int expect_length = 12;
	const char* test_fmt_string = "hello %+d";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_d_negative_with_positive)
{
	const char* expect = "hello -12345";
	int expect_length = 12;
	const char* test_fmt_string = "hello %+d";
	int test_value = -12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_d_negative)
{
	const char* expect = "hello -12345";
	int expect_length = 12;
	const char* test_fmt_string = "hello %d";
	int test_value = -12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_d_overflow)
{
	const char* expect = "hello -1294967296";
	int expect_length = 17;
	const char* test_fmt_string = "hello %d";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_i)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %i";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_i_negative)
{
	const char* expect = "hello -12345";
	int expect_length = 12;
	const char* test_fmt_string = "hello %i";
	int test_value = -12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_i_overflow)
{
	const char* expect = "hello -1294967296";
	int expect_length = 17;
	const char* test_fmt_string = "hello %i";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_u)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %u";
	unsigned int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_u_no_overflow)
{
	const char* expect = "hello 3000000000";
	int expect_length = 16;
	const char* test_fmt_string = "hello %u";
	unsigned int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_u_overflow)
{
	const char* expect = "hello 4294954951";
	int expect_length = 16;
	const char* test_fmt_string = "hello %u";
	unsigned int test_value = -12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_x)
{
	const char* expect = "hello 3039";
	int expect_length = 10;
	const char* test_fmt_string = "hello %x";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_x_no_overflow)
{
	const char* expect = "hello b2d05e00";
	int expect_length = 14;
	const char* test_fmt_string = "hello %x";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_X)
{
	const char* expect = "hello 3039";
	int expect_length = 10;
	const char* test_fmt_string = "hello %X";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_X_no_overflow)
{
	const char* expect = "hello B2D05E00";
	int expect_length = 14;
	const char* test_fmt_string = "hello %X";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_p)
{
	const char* expect = "hello 0x3ade68b1";
	int expect_length = 16;
	const char* test_fmt_string = "hello %p";
	void* test_value = (void*)987654321;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_s)
{
	const char* expect = "hello whatsup";
	int expect_length = 13;
	const char* test_fmt_string = "hello %s";
	char* test_value = (char*)"whatsup";
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_c)
{
	const char* expect = "hello x";
	int expect_length = 7;
	const char* test_fmt_string = "hello %c";
	char test_value = 'x';
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_d_with_0_padding)
{
	const char* expect = "hello 000000012345";
	int expect_length = 18;
	const char* test_fmt_string = "hello %012d";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_d_with_0_padding_undersized)
{
	const char* expect = "hello 2000000000";
	int expect_length = 16;
	const char* test_fmt_string = "hello %08d";
	int test_value = 2000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_i_with_0_padding)
{
	const char* expect = "hello 000000012345";
	int expect_length = 18;
	const char* test_fmt_string = "hello %012i";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_i_with_0_padding_undersized)
{
	const char* expect = "hello 2000000000";
	int expect_length = 16;
	const char* test_fmt_string = "hello %08i";
	int test_value = 2000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_u_with_0_padding)
{
	const char* expect = "hello 003000000000";
	int expect_length = 18;
	const char* test_fmt_string = "hello %012u";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_u_with_0_padding_undersized)
{
	const char* expect = "hello 3000000000";
	int expect_length = 16;
	const char* test_fmt_string = "hello %08u";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_x_with_0_padding)
{
	const char* expect = "hello 0000b2d05e00";
	int expect_length = 18;
	const char* test_fmt_string = "hello %012x";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_x_with_0_padding_undersized)
{
	const char* expect = "hello b2d05e00";
	int expect_length = 14;
	const char* test_fmt_string = "hello %08x";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}


TEST(test_printf, test_printf_percent_X_with_0_padding)
{
	const char* expect = "hello 0000B2D05E00";
	int expect_length = 18;
	const char* test_fmt_string = "hello %012X";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_X_with_0_padding_undersized)
{
	const char* expect = "hello B2D05E00";
	int expect_length = 14;
	const char* test_fmt_string = "hello %08X";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_p_with_0_padding)
{
	const char* expect = "hello 0x00003ade68b1";
	int expect_length = 20;
	const char* test_fmt_string = "hello %012p";
	void* test_value = (void*)987654321;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_p_with_0_padding_undersized)
{
	const char* expect = "hello 0x3ade68b1";
	int expect_length = 16;
	const char* test_fmt_string = "hello %08p";
	void* test_value = (void*)987654321;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_x_with_hash)
{
	const char* expect = "hello 0xb2d05e00";
	int expect_length = 16;
	const char* test_fmt_string = "hello %#x";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_x_with_hash_and_0_padding)
{
	const char* expect = "hello 0x0000b2d05e00";
	int expect_length = 20;
	const char* test_fmt_string = "hello %#012x";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_X_with_hash_and_0_padding)
{
	const char* expect = "hello 0x0000B2D05E00";
	int expect_length = 20;
	const char* test_fmt_string = "hello %#012X";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_X_with_hash)
{
	const char* expect = "hello 0xB2D05E00";
	int expect_length = 16;
	const char* test_fmt_string = "hello %#X";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}


TEST(test_printf, test_printf_percent_d_with_space_padding)
{
	const char* expect = "hello        12345";
	int expect_length = 18;
	const char* test_fmt_string = "hello % 12d";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_d_with_space_padding_undersized)
{
	const char* expect = "hello 2000000000";
	int expect_length = 16;
	const char* test_fmt_string = "hello % 8d";
	int test_value = 2000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_i_with_space_padding)
{
	const char* expect = "hello        12345";
	int expect_length = 18;
	const char* test_fmt_string = "hello % 12i";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_i_with_space_padding_undersized)
{
	const char* expect = "hello 2000000000";
	int expect_length = 16;
	const char* test_fmt_string = "hello % 8i";
	int test_value = 2000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_u_with_space_padding)
{
	const char* expect = "hello   3000000000";
	int expect_length = 18;
	const char* test_fmt_string = "hello % 12u";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_u_with_space_padding_undersized)
{
	const char* expect = "hello 3000000000";
	int expect_length = 16;
	const char* test_fmt_string = "hello % 8u";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_x_with_space_padding)
{
	const char* expect = "hello     b2d05e00";
	int expect_length = 18;
	const char* test_fmt_string = "hello % 12x";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_x_with_space_padding_undersized)
{
	const char* expect = "hello b2d05e00";
	int expect_length = 14;
	const char* test_fmt_string = "hello % 8x";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}


TEST(test_printf, test_printf_percent_X_with_space_padding)
{
	const char* expect = "hello     B2D05E00";
	int expect_length = 18;
	const char* test_fmt_string = "hello % 12X";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_X_with_space_padding_undersized)
{
	const char* expect = "hello B2D05E00";
	int expect_length = 14;
	const char* test_fmt_string = "hello % 8X";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_p_with_space_padding)
{
	const char* expect = "hello 0x    3ade68b1";
	int expect_length = 20;
	const char* test_fmt_string = "hello % 12p";
	void* test_value = (void*)987654321;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_p_with_space_padding_undersized)
{
	const char* expect = "hello 0x3ade68b1";
	int expect_length = 16;
	const char* test_fmt_string = "hello % 8p";
	void* test_value = (void*)987654321;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_x_with_hash_and_space_padding)
{
	const char* expect = "hello 0x    b2d05e00";
	int expect_length = 20;
	const char* test_fmt_string = "hello %# 12x";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_X_with_hash_and_space_padding)
{
	const char* expect = "hello 0x    B2D05E00";
	int expect_length = 20;
	const char* test_fmt_string = "hello %# 12X";
	int test_value = 3000000000;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_percent)
{
	const char* expect = "hello % %\n";
	int expect_length = 10;
	const char* test_fmt_string = "hello %% %%\n";
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_ld)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %ld";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_lld)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %lld";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_hd)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %hd";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_hhd)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %hhd";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}


TEST(test_printf, test_printf_percent_li)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %li";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_lli)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %lli";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_hi)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %hi";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_hhi)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %hhi";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}


TEST(test_printf, test_printf_percent_lu)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %lu";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_llu)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %llu";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_hu)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %hu";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_hhu)
{
	const char* expect = "hello 12345";
	int expect_length = 11;
	const char* test_fmt_string = "hello %hhu";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}


TEST(test_printf, test_printf_percent_lx)
{
	const char* expect = "hello 3039";
	int expect_length = 10;
	const char* test_fmt_string = "hello %lx";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_llx)
{
	const char* expect = "hello 3039";
	int expect_length = 10;
	const char* test_fmt_string = "hello %llx";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_hx)
{
	const char* expect = "hello 3039";
	int expect_length = 10;
	const char* test_fmt_string = "hello %hx";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

TEST(test_printf, test_printf_percent_hhx)
{
	const char* expect = "hello 3039";
	int expect_length = 10;
	const char* test_fmt_string = "hello %hhx";
	int test_value = 12345;
	int ret;

	reset_fixture();
	ret = printf(test_fmt_string, test_value);
	ASSERT_STREQ(expect, get_buffer());
	ASSERT_EQ(ret, expect_length);
}

/**
 * strutils
 */

TEST(test_strutils, strtoupper_tc_test_return_value_and_function)
{
	char* ret = NULL;
	char buf[] = "hello1123\t\n\r4567890!@#$%^&*\"()abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ{}:><?[]';./,\\|";
	ret = strtoupper(buf);
	ASSERT_STREQ((char*)"HELLO1123\t\n\r4567890!@#$%^&*\"()ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ{}:><?[]';./,\\|", buf);
	ASSERT_EQ(ret, buf);
}


TEST(test_strutils, strtolower_tc_test_return_value_and_function)
{
	char* ret = NULL;
	char buf[] = "hello1123\t\n\r4567890!@#$%^&*\"()abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ{}:><?[]';./,\\|";
	ret = strtolower(buf);
	ASSERT_STREQ((char*)"hello1123\t\n\r4567890!@#$%^&*\"()abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz{}:><?[]';./,\\|", buf);
	ASSERT_EQ(ret, buf);
}

TEST(test_strutils, adtoi_tc_test_return_value_and_function)
{
	ASSERT_EQ(adtoi('4'), 4);
	ASSERT_EQ(adtoi('B'), 11);
	ASSERT_EQ(adtoi('b'), 11);
	ASSERT_EQ(adtoi('-'), -1);
	ASSERT_EQ(adtoi('$'), -1);
}

TEST(test_strutils, ahtoi_tc_test_return_value_and_function)
{
	ASSERT_EQ(ahtoi((char*)"4"), 4);
	ASSERT_EQ(ahtoi((char*)"AB"), 171);
	ASSERT_EQ(ahtoi((char*)"ab"), 171);
	ASSERT_EQ(ahtoi((char*)"-AB"), -1);
	ASSERT_EQ(ahtoi((char*)"1234DEF"), 19090927);
}
