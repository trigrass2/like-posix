
#include "fixture.h"
#include "greenlight.h"
#include "minlibc/stdio.h"


TESTSUITE(test_printf)
{

}

TEST(test_printf, percent_d)
{
    int ret;

    reset_fixture();
    ret = printf("hello %d", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_printf, percent_d_with_positive)
{
    int ret;

    reset_fixture();
    ret = printf("hello %+d", 12345);
    ASSERT_STREQ((char*)"hello +12345", get_buffer());
    ASSERT_EQ(ret, 12);
}

TEST(test_printf, percent_d_negative_with_positive)
{
    int ret;

    reset_fixture();
    ret = printf("hello %+d", -12345);
    ASSERT_STREQ((char*)"hello -12345", get_buffer());
    ASSERT_EQ(ret, 12);
}

TEST(test_printf, percent_d_negative)
{
    int ret;

    reset_fixture();
    ret = printf("hello %d", -12345);
    ASSERT_STREQ((char*)"hello -12345", get_buffer());
    ASSERT_EQ(ret, 12);
}

TEST(test_printf, percent_d_overflow)
{
    int ret;

    reset_fixture();
    ret = printf("hello %d", (int)3000000000UL);
    ASSERT_STREQ((char*)"hello -1294967296", get_buffer());
    ASSERT_EQ(ret, 17);
}

TEST(test_printf, percent_i)
{
    int ret;

    reset_fixture();
    ret = printf("hello %i", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_printf, percent_i_negative)
{
    int ret;

    reset_fixture();
    ret = printf("hello %i", -12345);
    ASSERT_STREQ((char*)"hello -12345", get_buffer());
    ASSERT_EQ(ret, 12);
}

TEST(test_printf, percent_i_overflow)
{
    int ret;

    reset_fixture();
    ret = printf("hello %i", (int)3000000000UL);
    ASSERT_STREQ((char*)"hello -1294967296", get_buffer());
    ASSERT_EQ(ret, 17);
}

TEST(test_printf, percent_u)
{
    int ret;

    reset_fixture();
    ret = printf("hello %u", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_printf, percent_u_no_overflow)
{
    int ret;

    reset_fixture();
    ret = printf("hello %u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 3000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_u_overflow)
{
    int ret;

    reset_fixture();
    ret = printf("hello %u", -12345);
    ASSERT_STREQ((char*)"hello 4294954951", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_x)
{
    int ret;

    reset_fixture();
    ret = printf("hello %x", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_printf, percent_x_no_overflow)
{
    int ret;

    reset_fixture();
    ret = printf("hello %x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello b2d05e00", get_buffer());
    ASSERT_EQ(ret, 14);
}

TEST(test_printf, percent_X)
{
    int ret;

    reset_fixture();
    ret = printf("hello %X", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_printf, percent_X_no_overflow)
{
    int ret;

    reset_fixture();
    ret = printf("hello %X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello B2D05E00", get_buffer());
    ASSERT_EQ(ret, 14);
}

TEST(test_printf, percent_p)
{
    int ret;

    reset_fixture();
    ret = printf("hello %p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x3ade68b1", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_s)
{
    int ret;

    reset_fixture();
    ret = printf("hello %s", (char*)"whatsup");
    ASSERT_STREQ((char*)"hello whatsup", get_buffer());
    ASSERT_EQ(ret, 13);
}

TEST(test_printf, percent_c)
{
    int ret;

    reset_fixture();
    ret = printf("hello %c", 'x');
    ASSERT_STREQ((char*)"hello x", get_buffer());
    ASSERT_EQ(ret, 7);
}

TEST(test_printf, percent_d_with_0_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %012d", 12345);
    ASSERT_STREQ((char*)"hello 000000012345", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_d_with_0_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello %08d", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_i_with_0_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %012i", 12345);
    ASSERT_STREQ((char*)"hello 000000012345", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_i_with_0_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello %08i", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_u_with_0_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %012u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 003000000000", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_u_with_0_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello %08u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 3000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_x_with_0_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %012x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0000b2d05e00", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_x_with_0_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello %08x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello b2d05e00", get_buffer());
    ASSERT_EQ(ret, 14);
}


TEST(test_printf, percent_X_with_0_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %012X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0000B2D05E00", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_X_with_0_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello %08X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello B2D05E00", get_buffer());
    ASSERT_EQ(ret, 14);
}

TEST(test_printf, percent_p_with_0_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %012p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x00003ade68b1", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_printf, percent_p_with_0_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello %08p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x3ade68b1", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_x_with_hash)
{
    int ret;

    reset_fixture();
    ret = printf("hello %#x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0xb2d05e00", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_x_with_hash_and_0_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %#012x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x0000b2d05e00", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_printf, percent_X_with_hash_and_0_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %#012X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x0000B2D05E00", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_printf, percent_X_with_hash)
{
    int ret;

    reset_fixture();
    ret = printf("hello %#X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0xB2D05E00", get_buffer());
    ASSERT_EQ(ret, 16);
}


TEST(test_printf, percent_d_with_space_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello % 12d", 12345);
    ASSERT_STREQ((char*)"hello        12345", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_d_with_space_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello % 8d", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_i_with_space_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello % 12i", 12345);
    ASSERT_STREQ((char*)"hello        12345", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_i_with_space_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello % 8i", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_u_with_space_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello % 12u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello   3000000000", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_u_with_space_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello % 8u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 3000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_x_with_space_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello % 12x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello     b2d05e00", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_x_with_space_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello % 8x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello b2d05e00", get_buffer());
    ASSERT_EQ(ret, 14);
}


TEST(test_printf, percent_X_with_space_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello % 12X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello     B2D05E00", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_X_with_space_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello % 8X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello B2D05E00", get_buffer());
    ASSERT_EQ(ret, 14);
}

TEST(test_printf, percent_p_with_space_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello % 12p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x    3ade68b1", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_printf, percent_p_with_space_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello % 8p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x3ade68b1", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_x_with_hash_and_space_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %# 12x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x    b2d05e00", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_printf, percent_X_with_hash_and_space_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %# 12X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x    B2D05E00", get_buffer());
    ASSERT_EQ(ret, 20);
}

////////

TEST(test_printf, percent_d_with_empty_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %12d", 12345);
    ASSERT_STREQ((char*)"hello        12345", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_d_with_empty_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello %8d", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_i_with_empty_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %12i", 12345);
    ASSERT_STREQ((char*)"hello        12345", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_i_with_empty_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello %8i", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_u_with_empty_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %12u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello   3000000000", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_u_with_empty_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello %8u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 3000000000", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_x_with_empty_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %12x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello     b2d05e00", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_x_with_empty_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello %8x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello b2d05e00", get_buffer());
    ASSERT_EQ(ret, 14);
}


TEST(test_printf, percent_X_with_empty_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %12X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello     B2D05E00", get_buffer());
    ASSERT_EQ(ret, 18);
}

TEST(test_printf, percent_X_with_empty_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello %8X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello B2D05E00", get_buffer());
    ASSERT_EQ(ret, 14);
}

TEST(test_printf, percent_p_with_empty_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %12p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x    3ade68b1", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_printf, percent_p_with_empty_padding_undersized)
{
    int ret;

    reset_fixture();
    ret = printf("hello %8p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x3ade68b1", get_buffer());
    ASSERT_EQ(ret, 16);
}

TEST(test_printf, percent_x_with_hash_and_empty_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %#12x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x    b2d05e00", get_buffer());
    ASSERT_EQ(ret, 20);
}

TEST(test_printf, percent_X_with_hash_and_empty_padding)
{
    int ret;

    reset_fixture();
    ret = printf("hello %#12X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x    B2D05E00", get_buffer());
    ASSERT_EQ(ret, 20);
}
///////

TEST(test_printf, percent_percent)
{   int ret;

    reset_fixture();
    ret = printf("hello %% %%\n");
    ASSERT_STREQ((char*)"hello % %\n", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_printf, percent_ld)
{
    int ret;

    reset_fixture();
    ret = printf("hello %ld", (long int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_printf, percent_lld)
{
    int ret;

    reset_fixture();
    ret = printf("hello %lld", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_printf, percent_hd)
{
    int ret;

    reset_fixture();
    ret = printf("hello %hd", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_printf, percent_hhd)
{
    int ret;

    reset_fixture();
    ret = printf("hello %hhd", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}


TEST(test_printf, percent_li)
{
    int ret;

    reset_fixture();
    ret = printf("hello %li", (long int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_printf, percent_lli)
{
    int ret;

    reset_fixture();
    ret = printf("hello %lli", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_printf, percent_hi)
{
    int ret;

    reset_fixture();
    ret = printf("hello %hi", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_printf, percent_hhi)
{
    int ret;

    reset_fixture();
    ret = printf("hello %hhi", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}


TEST(test_printf, percent_lu)
{
    int ret;

    reset_fixture();
    ret = printf("hello %lu", (long unsigned int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_printf, percent_llu)
{
    int ret;

    reset_fixture();
    ret = printf("hello %llu", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_printf, percent_hu)
{
    int ret;

    reset_fixture();
    ret = printf("hello %hu", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}

TEST(test_printf, percent_hhu)
{
    int ret;

    reset_fixture();
    ret = printf("hello %hhu", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
}


TEST(test_printf, percent_lx)
{
    int ret;

    reset_fixture();
    ret = printf("hello %lx", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_printf, percent_llx)
{
    int ret;

    reset_fixture();
    ret = printf("hello %llx", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_printf, percent_hx)
{
    int ret;

    reset_fixture();
    ret = printf("hello %hx", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_printf, percent_hhx)
{
    int ret;

    reset_fixture();
    ret = printf("hello %hhx", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
}

TEST(test_printf, percent_f)
{
    int ret;
    float fl = 1.0;
    int i;

    for(i = 0; i < 10; i += 1)
    {
        reset_fixture();
        ret = printf("hello %f", fl);
        ASSERT_STREQ((char*)"hello 1.1", get_buffer());
//        ASSERT_EQ(ret, 16);

        fl += 0.1;
    }
}
