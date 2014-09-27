
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "gtest/gtest.h"
#include "minstdio.h"

char phy_putc_buf[1024];
uint32_t i = 0;
void phy_putc(char c)
{
	phy_putc_buf[i] = c;
	i++;
	phy_putc_buf[i] = 0;
}

TEST(sprintf_tc, test_decimal_formatting)
{
	char buf[1024];
	int ret = sprintf(buf, "hello %d", 12345);
	ASSERT_STREQ((char*)"hello 12345", buf);
	ASSERT_EQ(ret, 11);
}

TEST(printf_tc, test_decimal_formatting)
{
	i = 0;
	int ret = printf("hello %d", 12345);
	ASSERT_STREQ((char*)"hello 12345", phy_putc_buf);
	ASSERT_EQ(ret, 11);
}
