
#include "fixture.h"
#include "greenlight.h"
#include "minlibc/stdio.h"
#include <string.h>

TESTSUITE(test_fprintf)
{

}

TEST(test_fprintf, test_init)
{
	init_minlibc();
}

TEST(test_fprintf, percent_d)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %d", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
	fclose(f);
}

TEST(test_fprintf, percent_d_with_positive)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    ASSERT_NEQ((intptr_t)f, (intptr_t)NULL);

    reset_fixture();
    ret = fprintf(f, "hello %+d", 12345);
    ASSERT_STREQ((char*)"hello +12345", get_buffer());
    ASSERT_EQ(ret, 12);
	fclose(f);
}

TEST(test_fprintf, percent_d_negative_with_positive)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %+d", -12345);
    ASSERT_STREQ((char*)"hello -12345", get_buffer());
    ASSERT_EQ(ret, 12);
	fclose(f);
}

TEST(test_fprintf, percent_d_negative)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %d", -12345);
    ASSERT_STREQ((char*)"hello -12345", get_buffer());
    ASSERT_EQ(ret, 12);
	fclose(f);
}

TEST(test_fprintf, percent_d_overflow)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %d", (int)3000000000UL);
    ASSERT_STREQ((char*)"hello -1294967296", get_buffer());
    ASSERT_EQ(ret, 17);
	fclose(f);
}

TEST(test_fprintf, percent_i)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %i", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
	fclose(f);
}

TEST(test_fprintf, percent_i_negative)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %i", -12345);
    ASSERT_STREQ((char*)"hello -12345", get_buffer());
    ASSERT_EQ(ret, 12);
	fclose(f);
}

TEST(test_fprintf, percent_i_overflow)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %i", (int)3000000000UL);
    ASSERT_STREQ((char*)"hello -1294967296", get_buffer());
    ASSERT_EQ(ret, 17);
	fclose(f);
}

TEST(test_fprintf, percent_u)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %u", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
	fclose(f);
}

TEST(test_fprintf, percent_u_no_overflow)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 3000000000", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_u_overflow)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %u", -12345);
    ASSERT_STREQ((char*)"hello 4294954951", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_x)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %x", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
	fclose(f);
}

TEST(test_fprintf, percent_x_no_overflow)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello b2d05e00", get_buffer());
    ASSERT_EQ(ret, 14);
	fclose(f);
}

TEST(test_fprintf, percent_X)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %X", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);
	fclose(f);
}

TEST(test_fprintf, percent_X_no_overflow)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello B2D05E00", get_buffer());
    ASSERT_EQ(ret, 14);
	fclose(f);
}

TEST(test_fprintf, percent_p)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x3ade68b1", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_s)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %s", (char*)"whatsup");
    ASSERT_STREQ((char*)"hello whatsup", get_buffer());
    ASSERT_EQ(ret, 13);
	fclose(f);
}

TEST(test_fprintf, percent_c)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %c", 'x');
    ASSERT_STREQ((char*)"hello x", get_buffer());
    ASSERT_EQ(ret, 7);
	fclose(f);
}

TEST(test_fprintf, percent_d_with_0_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %012d", 12345);
    ASSERT_STREQ((char*)"hello 000000012345", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_d_with_0_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %08d", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_i_with_0_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %012i", 12345);
    ASSERT_STREQ((char*)"hello 000000012345", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_i_with_0_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %08i", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_u_with_0_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %012u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 003000000000", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_u_with_0_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %08u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 3000000000", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_x_with_0_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %012x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0000b2d05e00", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_x_with_0_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %08x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello b2d05e00", get_buffer());
    ASSERT_EQ(ret, 14);
	fclose(f);
}


TEST(test_fprintf, percent_X_with_0_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %012X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0000B2D05E00", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_X_with_0_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %08X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello B2D05E00", get_buffer());
    ASSERT_EQ(ret, 14);
	fclose(f);
}

TEST(test_fprintf, percent_p_with_0_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %012p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x00003ade68b1", get_buffer());
    ASSERT_EQ(ret, 20);
	fclose(f);
}

TEST(test_fprintf, percent_p_with_0_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %08p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x3ade68b1", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_x_with_hash)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %#x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0xb2d05e00", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_x_with_hash_and_0_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %#012x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x0000b2d05e00", get_buffer());
    ASSERT_EQ(ret, 20);
	fclose(f);
}

TEST(test_fprintf, percent_X_with_hash_and_0_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %#012X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x0000B2D05E00", get_buffer());
    ASSERT_EQ(ret, 20);
	fclose(f);
}

TEST(test_fprintf, percent_X_with_hash)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %#X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0xB2D05E00", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}


TEST(test_fprintf, percent_d_with_space_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello % 12d", 12345);
    ASSERT_STREQ((char*)"hello        12345", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_d_with_space_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello % 8d", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_i_with_space_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello % 12i", 12345);
    ASSERT_STREQ((char*)"hello        12345", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_i_with_space_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello % 8i", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_u_with_space_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello % 12u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello   3000000000", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_u_with_space_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello % 8u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 3000000000", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_x_with_space_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello % 12x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello     b2d05e00", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_x_with_space_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello % 8x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello b2d05e00", get_buffer());
    ASSERT_EQ(ret, 14);
	fclose(f);
}


TEST(test_fprintf, percent_X_with_space_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello % 12X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello     B2D05E00", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_X_with_space_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello % 8X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello B2D05E00", get_buffer());
    ASSERT_EQ(ret, 14);
	fclose(f);
}

TEST(test_fprintf, percent_p_with_space_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello % 12p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x    3ade68b1", get_buffer());
    ASSERT_EQ(ret, 20);
	fclose(f);
}

TEST(test_fprintf, percent_p_with_space_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello % 8p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x3ade68b1", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_x_with_hash_and_space_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %# 12x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x    b2d05e00", get_buffer());
    ASSERT_EQ(ret, 20);
	fclose(f);
}

TEST(test_fprintf, percent_X_with_hash_and_space_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %# 12X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x    B2D05E00", get_buffer());
    ASSERT_EQ(ret, 20);
	fclose(f);
}

///

TEST(test_fprintf, percent_d_with_empty_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %12d", 12345);
    ASSERT_STREQ((char*)"hello        12345", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_d_with_empty_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %8d", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_i_with_empty_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %12i", 12345);
    ASSERT_STREQ((char*)"hello        12345", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_i_with_empty_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %8i", 2000000000);
    ASSERT_STREQ((char*)"hello 2000000000", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_u_with_empty_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %12u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello   3000000000", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_u_with_empty_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %8u", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 3000000000", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_x_with_empty_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %12x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello     b2d05e00", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_x_with_empty_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %8x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello b2d05e00", get_buffer());
    ASSERT_EQ(ret, 14);
	fclose(f);
}


TEST(test_fprintf, percent_X_with_empty_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %12X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello     B2D05E00", get_buffer());
    ASSERT_EQ(ret, 18);
	fclose(f);
}

TEST(test_fprintf, percent_X_with_empty_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %8X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello B2D05E00", get_buffer());
    ASSERT_EQ(ret, 14);
	fclose(f);
}

TEST(test_fprintf, percent_p_with_empty_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %12p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x    3ade68b1", get_buffer());
    ASSERT_EQ(ret, 20);
	fclose(f);
}

TEST(test_fprintf, percent_p_with_empty_padding_undersized)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %8p", (uintptr_t)987654321);
    ASSERT_STREQ((char*)"hello 0x3ade68b1", get_buffer());
    ASSERT_EQ(ret, 16);
	fclose(f);
}

TEST(test_fprintf, percent_x_with_hash_and_empty_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %#12x", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x    b2d05e00", get_buffer());
    ASSERT_EQ(ret, 20);
	fclose(f);
}

TEST(test_fprintf, percent_X_with_hash_and_empty_padding)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %#12X", (unsigned int)3000000000UL);
    ASSERT_STREQ((char*)"hello 0x    B2D05E00", get_buffer());
    ASSERT_EQ(ret, 20);
	fclose(f);
}
///

TEST(test_fprintf, percent_percent)
{   int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %% %%\n");
    ASSERT_STREQ((char*)"hello % %\n", get_buffer());
    ASSERT_EQ(ret, 10);
	fclose(f);
}

TEST(test_fprintf, percent_ld)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %ld", (long int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
	fclose(f);
}

TEST(test_fprintf, percent_lld)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %lld", (long long int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
	fclose(f);
}

TEST(test_fprintf, percent_hd)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %hd", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
	fclose(f);
}

TEST(test_fprintf, percent_hhd)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %hhd", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
	fclose(f);
}


TEST(test_fprintf, percent_li)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %li", (long int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
	fclose(f);
}

TEST(test_fprintf, percent_lli)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %lli", (long long int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
	fclose(f);
}

TEST(test_fprintf, percent_hi)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %hi", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
	fclose(f);
}

TEST(test_fprintf, percent_hhi)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %hhi", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
	fclose(f);
}


TEST(test_fprintf, percent_lu)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %lu", (long unsigned int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);
	fclose(f);
}

TEST(test_fprintf, percent_llu)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %llu", (long long unsigned int)12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);

	fclose(f);
}

TEST(test_fprintf, percent_hu)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %hu", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);

	fclose(f);
}

TEST(test_fprintf, percent_hhu)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %hhu", 12345);
    ASSERT_STREQ((char*)"hello 12345", get_buffer());
    ASSERT_EQ(ret, 11);

	fclose(f);
}


TEST(test_fprintf, percent_lx)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %lx", (long unsigned int)12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);

	fclose(f);
}

TEST(test_fprintf, percent_llx)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %llx", (long long unsigned int)12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);

    fclose(f);
}

TEST(test_fprintf, percent_hx)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %hx", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);

    fclose(f);
}

TEST(test_fprintf, percent_hhx)
{
    int ret;
    FILE* f = fopen("test.txt", "w");

    reset_fixture();
    ret = fprintf(f, "hello %hhx", 12345);
    ASSERT_STREQ((char*)"hello 3039", get_buffer());
    ASSERT_EQ(ret, 10);

    fclose(f);
}
