
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "gtest/gtest.h"
#include "strutils.h"


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

TEST(test_confparse, test_string_in_list)
{
    bool ret = string_in_list((const uint8_t*)"abc", 10, (const uint8_t**)valid_match_set);
    ASSERT_TRUE(ret);
    ret = string_in_list((const uint8_t*)"efg", 10, (const uint8_t**)valid_match_set);
    ASSERT_TRUE(ret);
    ret = string_in_list((const uint8_t*)"xyz", 10, (const uint8_t**)valid_match_set);
    ASSERT_TRUE(ret);
    ret = string_in_list((const uint8_t*)"gfd", 10, (const uint8_t**)valid_match_set);
    ASSERT_FALSE(ret);
    ret = string_in_list((const uint8_t*)"ab", 10, (const uint8_t**)valid_match_set);
    ASSERT_FALSE(ret);
    ret = string_in_list((const uint8_t*)"zy", 10, (const uint8_t**)valid_match_set);
    ASSERT_FALSE(ret);
}

TEST(test_confparse, test_string_match)
{
    bool ret = string_match("abc", (const uint8_t*)"abc");
    ASSERT_TRUE(ret);
    ret = string_match("abc", (const uint8_t*)"abcd");
    ASSERT_FALSE(ret);
    ret = string_match("abcd", (const uint8_t*)"abc");
    ASSERT_FALSE(ret);
    ret = string_match(NULL, (const uint8_t*)"abc");
    ASSERT_FALSE(ret);
    ret = string_match("abcd", NULL);
    ASSERT_FALSE(ret);
}
