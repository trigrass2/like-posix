
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "gtest/gtest.h"
#include "strutils.h"


TEST(strtoupper_tc, test_return_value_and_function)
{
	char* ret = NULL;
	char buf[] = "hello1123\t\n\r4567890!@#$%^&*\"()abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ{}:><?[]';./,\\|";
	ret = strtoupper(buf);
	ASSERT_STREQ((char*)"HELLO1123\t\n\r4567890!@#$%^&*\"()ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ{}:><?[]';./,\\|", buf);
	ASSERT_EQ(ret, buf);
}


TEST(strtolower_tc, test_return_value_and_function)
{
	char* ret = NULL;
	char buf[] = "hello1123\t\n\r4567890!@#$%^&*\"()abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ{}:><?[]';./,\\|";
	ret = strtolower(buf);
	ASSERT_STREQ((char*)"hello1123\t\n\r4567890!@#$%^&*\"()abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz{}:><?[]';./,\\|", buf);
	ASSERT_EQ(ret, buf);
}

TEST(adtoi_tc, test_return_value_and_function)
{
	ASSERT_EQ(adtoi('4'), 4);
	ASSERT_EQ(adtoi('B'), 11);
	ASSERT_EQ(adtoi('b'), 11);
	ASSERT_EQ(adtoi('-'), -1);
	ASSERT_EQ(adtoi('$'), -1);
}

TEST(ahtoi_tc, test_return_value_and_function)
{
	ASSERT_EQ(ahtoi((char*)"4"), 4);
	ASSERT_EQ(ahtoi((char*)"AB"), 171);
	ASSERT_EQ(ahtoi((char*)"ab"), 171);
	ASSERT_EQ(ahtoi((char*)"-AB"), -1);
	ASSERT_EQ(ahtoi((char*)"1234DEF"), 19090927);
}
