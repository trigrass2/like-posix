
#include "fixture.h"
#include "greenlight.h"
#include "minlibc/stdio.h"

TESTSUITE(test_ffunc)
{

}

TEST(test_ffunc, test_init)
{
	init_minlibc();
}

TEST(test_ffunc, test_fclose_stdout)
{
    int ret;
    reset_fixture();
    ret = fclose(stdout);
    ASSERT_EQ(ret, EOF);
}

TEST(test_ffunc, test_fclose_stderr)
{
    int ret;
    reset_fixture();
    ret = fclose(stderr);
    ASSERT_EQ(ret, EOF);
}

TEST(test_ffunc, test_fputs)
{
    int ret;
    FILE* f = fopen("test0.txt", "w");

    reset_fixture();
    ret = fputs("hello 123", f);
    ASSERT_STREQ((char*)"hello 123", get_buffer());
    ASSERT_EQ(ret, 9);

    fclose(f);
}

TEST(test_ffunc, test_fputs_stdout)
{
    int ret;
    reset_fixture();
    ret = fputs("hello 123", stdout);
    ASSERT_STREQ((char*)"hello 123", get_buffer());
    ASSERT_EQ(ret, 9);
}

TEST(test_ffunc, test_fputs_stderr)
{
    int ret;
    reset_fixture();
    ret = fputs("hello 123", stderr);
    ASSERT_STREQ((char*)"hello 123", get_buffer());
    ASSERT_EQ(ret, 9);
}

TEST(test_ffunc, test_fputs_null)
{
    int ret;
    reset_fixture();
    ret = fputs(get_buffer(), NULL);
    ASSERT_EQ(ret, EOF);
}

TEST(test_ffunc, test_fputc)
{
    int ret;
    FILE* fd = fopen("test1.txt", "w");
    reset_fixture();
    ASSERT_EQ(get_buffer()[0], '\0');
    ret = fputc('5', fd);
    ASSERT_EQ((char)ret, '5');
    ASSERT_EQ(get_buffer()[0], '5');
    fclose(fd);
}

TEST(test_ffunc, test_fgets_null_fd)
{
    char* ret;
    reset_fixture();
    ret = fgets(get_buffer(), BUFFER_SIZE, NULL);
    ASSERT_EQ((intptr_t)ret, NULL);
}

TEST(test_ffunc, test_fgets_no_newline)
{
    char buf[100];
    const char* expect = "hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234hello 1234";

    char* ret;
    FILE* fd = fopen("test2.txt", "r");

    reset_fixture();
    strcpy(get_buffer(), expect);

    ret = fgets(buf, sizeof(buf), fd);

    ASSERT_EQ((intptr_t)ret, (intptr_t)buf);
//  ASSERT_STREQ(expect, buf); // buf is truncated to 99 chars long
    ASSERT_EQ((int)strlen(buf), (int)(sizeof(buf)-1));

    fclose(fd);
}

TEST(test_ffunc, test_fgets_with_newline)
{
    char buf[100];
    const char* expect = "hello 123\nwerwerwerwerwer";

    char* ret;
    FILE* fd = fopen("test3.txt", "r");

    reset_fixture();
    strcpy(get_buffer(), expect);

    ret = fgets(buf, sizeof(buf), fd);

    ASSERT_EQ((intptr_t)ret, (intptr_t)buf);
    ASSERT_STREQ((char*)"hello 123\n", buf);
    ASSERT_EQ((int)strlen(buf), (int)strlen("hello 123\n"));

    fclose(fd);
}

TEST(test_ffunc, test_fgetc)
{
    int ret;
    FILE* fd = fopen("test4.txt", "r");

    ASSERT_EQ((int)(((fake__FILE*)fd)->_file), (int)3);
    ASSERT_NEQ((intptr_t)fd, (intptr_t)NULL);

    reset_fixture();
    force_eof();
    ret = fgetc(fd);
    ASSERT_EQ(EOF, ret);

    reset_fixture();
    get_buffer()[0] = '5';
    ret = fgetc(fd);
    ASSERT_EQ((int)'5', ret);

    fclose(fd);
}

TEST(test_ffunc, test_fgetc_ungetc)
{
    int ret;
    FILE* fd = fopen("test4.txt", "r");

    ASSERT_NEQ((intptr_t)fd, (intptr_t)NULL);
    reset_fixture();
    get_buffer()[0] = '5';
    ret = fgetc(fd);
    ASSERT_EQ((int)'5', ret);
    ungetc('6', fd);
    ret = fgetc(fd);
    ASSERT_EQ((int)'6', ret);

    fclose(fd);
}
