
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "gtest/gtest.h"
#include "strutils.h"

const char* valid_match_set[] = {
        "abc",
        "123",
        "efg",
        "456",
        "xyz",
        "987",
        NULL
};

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
	ASSERT_EQ(adtoi('4'), (char)4);
	ASSERT_EQ(adtoi('B'), (char)11);
	ASSERT_EQ(adtoi('b'), (char)11);
	ASSERT_EQ(adtoi('-'), (char)-1);
	ASSERT_EQ(adtoi('$'), (char)-1);
}

TEST(test_strutils, ahtoi_tc_test_return_value_and_function)
{
	ASSERT_EQ(ahtoi((char*)"4"), (int)4);
	ASSERT_EQ(ahtoi((char*)"AB"), (int)171);
	ASSERT_EQ(ahtoi((char*)"ab"), (int)171);
	ASSERT_EQ(ahtoi((char*)"-AB"), (int)-1);
	ASSERT_EQ(ahtoi((char*)"1234DEF"), (int)19090927);
}

TEST(test_strutils, test_string_in_list)
{
    int ret = string_in_list("abc", sizeof("abc"), valid_match_set);
    ASSERT_EQ(ret, 0);
    ret = string_in_list("efg", sizeof("efg")-1, valid_match_set);
    ASSERT_EQ(ret, 2);
    ret = string_in_list("xyz", sizeof("xyz")-1, valid_match_set);
    ASSERT_EQ(ret, 4);
    ret = string_in_list("gfd", sizeof("gfd")-1, valid_match_set);
    ASSERT_EQ(ret, -1);
    ret = string_in_list("ab", 10, valid_match_set);
    ASSERT_EQ(ret, -1);
    ret = string_in_list("zy", 10, valid_match_set);
    ASSERT_EQ(ret, -1);
}
