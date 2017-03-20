
#include "fixture.h"
#include "greenlight.h"
#include "minlibc/stdio.h"

TESTSUITE(test_sprintf)
{

}

TEST(test_sprintf, percent_d)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %d", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, percent_d_with_positive)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %+d", 12345);
    ASSERT_STREQ((char*)"hello +12345", get_buffer());
    ASSERT_EQ(ret, 12);
}

TEST(test_sprintf, percent_d_negative_with_positive)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %+d", -12345);
    ASSERT_STREQ((char*)"hello -12345", get_buffer());
    ASSERT_EQ(ret, 12);
}

TEST(test_sprintf, percent_d_negative)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %d", -12345);
    ASSERT_STREQ((char*)"hello -12345", get_buffer());
    ASSERT_EQ(ret, 12);
}

TEST(test_sprintf, percent_d_overflow)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %d", (int)3000000000UL);
    ASSERT_STREQ((char*)"hello -1294967296", get_buffer());
    ASSERT_EQ(ret, 17);
}

TEST(test_sprintf, percent_i)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %i", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, percent_i_negative)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %i", -12345);
    ASSERT_STREQ((char*)"hello -12345", get_buffer());
    ASSERT_EQ(ret, 12);
}

TEST(test_sprintf, percent_i_overflow)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %i", (int)3000000000UL);
    ASSERT_STREQ((char*)"hello -1294967296", get_buffer());
    ASSERT_EQ(ret, 17);
}

TEST(test_sprintf, percent_u)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %u", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, percent_u_no_overflow)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 3000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_u_overflow)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %u", -12345);
    ASSERT_STREQ((char*)"hello 4294954951", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_x)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %x", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_sprintf, percent_x_no_overflow)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello b2d05e00", get_buffer());
    ASSERT_EQ(ret, 14);
}

TEST(test_sprintf, percent_X)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %X", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_sprintf, percent_X_no_overflow)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello B2D05E00", get_buffer());
    ASSERT_EQ(ret, 14);
}

TEST(test_sprintf, percent_p)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x3ade68b1", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_s)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %s", (char*)"whatsup");
    ASSERT_STREQ((char*)"hello whatsup", get_buffer());
    ASSERT_EQ(ret, 13);
}

TEST(test_sprintf, percent_c)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %c", 'x');
    ASSERT_STREQ((char*)"hello x", get_buffer());
    ASSERT_EQ(ret, 7);
}

TEST(test_sprintf, percent_d_with_0_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %012d", 12345);
    ASSERT_STREQ((char*)"hello 000000012345", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_d_with_0_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %08d", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_i_with_0_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %012i", 12345);
    ASSERT_STREQ((char*)"hello 000000012345", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_i_with_0_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %08i", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_u_with_0_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %012u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 003000000000", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_u_with_0_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %08u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 3000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_x_with_0_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %012x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0000b2d05e00", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_x_with_0_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %08x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello b2d05e00", get_buffer());
    ASSERT_EQ(ret, 14);
}


TEST(test_sprintf, percent_X_with_0_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %012X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0000B2D05E00", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_X_with_0_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %08X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello B2D05E00", get_buffer());
    ASSERT_EQ(ret, 14);
}

TEST(test_sprintf, percent_p_with_0_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %012p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x00003ade68b1", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, percent_p_with_0_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %08p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x3ade68b1", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_x_with_hash)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %#x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0xb2d05e00", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_x_with_hash_and_0_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %#012x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x0000b2d05e00", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, percent_X_with_hash_and_0_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %#012X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x0000B2D05E00", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, percent_X_with_hash)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %#X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0xB2D05E00", get_buffer());
    ASSERT_EQ(ret, 16);
}


TEST(test_sprintf, percent_d_with_space_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello % 12d", 12345);
    ASSERT_STREQ((char*)"hello        12345", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_d_with_space_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello % 8d", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_i_with_space_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello % 12i", 12345);
    ASSERT_STREQ((char*)"hello        12345", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_i_with_space_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello % 8i", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_u_with_space_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello % 12u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello   3000000000", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_u_with_space_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello % 8u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 3000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_x_with_space_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello % 12x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello     b2d05e00", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_x_with_space_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello % 8x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello b2d05e00", get_buffer());
    ASSERT_EQ(ret, 14);
}


TEST(test_sprintf, percent_X_with_space_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello % 12X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello     B2D05E00", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_X_with_space_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello % 8X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello B2D05E00", get_buffer());
    ASSERT_EQ(ret, 14);
}

TEST(test_sprintf, percent_p_with_space_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello % 12p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x    3ade68b1", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, percent_p_with_space_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello % 8p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x3ade68b1", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_x_with_hash_and_space_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %# 12x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x    b2d05e00", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, percent_X_with_hash_and_space_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %# 12X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x    B2D05E00", get_buffer());
    ASSERT_EQ(ret, 20);
}

///

TEST(test_sprintf, percent_d_with_empty_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %12d", 12345);
    ASSERT_STREQ((char*)"hello        12345", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_d_with_empty_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %8d", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_i_with_empty_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %12i", 12345);
    ASSERT_STREQ((char*)"hello        12345", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_i_with_empty_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %8i", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_u_with_empty_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %12u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello   3000000000", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_u_with_empty_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %8u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 3000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_x_with_empty_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %12x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello     b2d05e00", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_x_with_empty_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %8x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello b2d05e00", get_buffer());
    ASSERT_EQ(ret, 14);
}


TEST(test_sprintf, percent_X_with_empty_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %12X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello     B2D05E00", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_sprintf, percent_X_with_empty_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %8X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello B2D05E00", get_buffer());
    ASSERT_EQ(ret, 14);
}

TEST(test_sprintf, percent_p_with_empty_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %12p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x    3ade68b1", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, percent_p_with_empty_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %8p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x3ade68b1", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_sprintf, percent_x_with_hash_and_empty_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %#12x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x    b2d05e00", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_sprintf, percent_X_with_hash_and_empty_padding)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %#12X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x    B2D05E00", get_buffer());
    ASSERT_EQ(ret, 20);
}
///
TEST(test_sprintf, percent_percent)
{   int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %% %%\n");
    ASSERT_STREQ((char*)"hello % %\n", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_sprintf, percent_ld)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %ld", (long int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, percent_lld)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %lld", (long long int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, percent_hd)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %hd", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, percent_hhd)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %hhd", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}


TEST(test_sprintf, percent_li)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %li", (long int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, percent_lli)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %lli", (long long int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, percent_hi)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %hi", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, percent_hhi)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %hhi", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}


TEST(test_sprintf, percent_lu)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %lu", (long unsigned int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, percent_llu)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %llu", (long long unsigned int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, percent_hu)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %hu", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_sprintf, percent_hhu)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %hhu", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}


TEST(test_sprintf, percent_lx)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %lx", (long unsigned int)12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_sprintf, percent_llx)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %llx", (long long unsigned int)12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_sprintf, percent_hx)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %hx", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_sprintf, percent_hhx)
{
    int ret;

    reset_fixture();
    ret = sprintf(get_buffer(), "hello %hhx", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
}

