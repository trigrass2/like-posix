
#include <stdint.h>
#include <limits.h>
#include "gtest/gtest.h"
#include "strutils.h"


/********************
 * test fixture
 */
char errmsg[1024];

void raise_error(const uint8_t* msg, const uint8_t* function)
{
	sprintf(errmsg, "error: %s in function %s", msg, function);
}

void reset_errmsg()
{
	errmsg[0] = '\0';
}

/********************
 * test cases
 */

TEST(vfifo_init_tc, with_zero_length_and_null_buffer)
{
//	vfifo_t f;
//	void** buf = NULL;
	reset_errmsg();
//	vfifo_init(&f, buf, 0);
	ASSERT_STREQ("", errmsg);
//	ASSERT_EQ(f.size, 0);
}
//
//TEST(vfifo_init_tc, with_zero_length_and_real_buffer)
//{
//	vfifo_t f;
//	void* buf[10];
//	reset_errmsg();
//	vfifo_init(&f, buf, 0);
//	ASSERT_STREQ("", errmsg);
//	ASSERT_EQ(f.size, 0);
//}
//
//TEST(vfifo_init_tc, with_non_zero_length_and_null_buffer_raises_error)
//{
//	vfifo_t f;
//	void** buf = NULL;
//	reset_errmsg();
//	vfifo_init(&f, buf, 10);
//	ASSERT_STREQ("error: buffer set to null in function vfifo_init", errmsg);
//	ASSERT_EQ(f.size, 0);
//}
//
//TEST(vfifo_init_tc, with_non_zero_length_and_real_buffer)
//{
//	vfifo_t f;
//	void* buf[10];
//	uint32_t size = sizeof(buf)/sizeof(void*);
//	reset_errmsg();
//	vfifo_init(&f, buf, size);
//	ASSERT_STREQ("", errmsg);
//	ASSERT_EQ(f.size, size);
//}
//
//TEST(vfifo_put_tc, with_non_zero_length_and_real_buffer_succeeds)
//{
//	void* pointer = (void*)vfifo_put;
//	vfifo_t f;
//	void* buf[10] = {NULL};
//	vfifo_init(&f, buf, sizeof(buf)/sizeof(void*));
//	ASSERT_NE(f.buf[0], pointer);
//	ASSERT_TRUE(vfifo_put(&f, pointer));
//	ASSERT_EQ(f.buf[0], pointer);
//}
//
//TEST(vfifo_put_tc, with_zero_length_put_fails)
//{
//	void* pointer = (void*)vfifo_put;
//	vfifo_t f;
//	void* buf[10] = {NULL};
//	vfifo_init(&f, buf, 0);
//	ASSERT_NE(f.buf[0], pointer);
//	ASSERT_FALSE(vfifo_put(&f, pointer));
//	ASSERT_NE(f.buf[0], pointer);
//}
//
//
//TEST(vfifo_put_tc, with_non_zero_length_put_succeeds_on_overflow_fails)
//{
//	uint8_t i;
//	void* pointer = (void*)vfifo_put;
//	vfifo_t f;
//	// set up buffer 4 times oversized
//	void* buf[40] = {NULL};
//	// set fifo size to 1/4 buffer size
//	uint32_t size = sizeof(buf)/sizeof(void*)/4;
//
//	vfifo_init(&f, buf, size);
//	// test sucessful entries (we only get size-1 free slots in the buffer)
//	for(i=0; i < size-1; i++)
//	{
//		ASSERT_NE(f.buf[i], pointer);
//		ASSERT_TRUE(vfifo_put(&f, pointer));
//		ASSERT_EQ(f.buf[i], pointer);
//	}
//	// test unsuccessful entries
//	for(; i < size*4; i++)
//	{
//		ASSERT_FALSE(vfifo_put(&f, pointer));
//		ASSERT_NE(f.buf[i], pointer);
//	}
//}
//
//TEST(vfifo_get_tc, with_non_zero_length_get_succeeds_on_underflow_fails)
//{
//	void* pointer1 = (void*)vfifo_get;
//	void* pointer2 = (void*)vfifo_put;
//	void* pointer3 = (void*)NULL;
//	void* pointer4 = (void*)NULL;
//
//	vfifo_t f;
//	void* buf[10] = {NULL};
//	vfifo_init(&f, buf, sizeof(buf)/sizeof(void*));
//
//	vfifo_put(&f, pointer1);
//	vfifo_put(&f, pointer2);
//	ASSERT_TRUE(vfifo_get(&f, &pointer3));
//	ASSERT_TRUE(vfifo_get(&f, &pointer4));
//	ASSERT_EQ(pointer1, pointer3);
//	ASSERT_EQ(pointer2, pointer4);
//	pointer3 = NULL;
//	pointer4 = NULL;
//	ASSERT_FALSE(vfifo_get(&f, &pointer3));
//	ASSERT_FALSE(vfifo_get(&f, &pointer4));
//	ASSERT_EQ(pointer3, (void*)NULL);
//	ASSERT_EQ(pointer4, (void*)NULL);
//}
//
//TEST(vfifo_usage_tc, starts_at_0_with_non_zero_length)
//{
//	vfifo_t f;
//	void* buf[10] = {NULL};
//	vfifo_init(&f, buf, sizeof(buf)/sizeof(void*));
//	ASSERT_EQ(vfifo_usage(&f), 0);
//}
//
//TEST(vfifo_usage_tc, tracks_correctly)
//{
//	uint8_t i;
//	void* pointer = (void*)vfifo_get;
//	vfifo_t f;
//	void* buf[10] = {NULL};
//	uint8_t size = sizeof(buf)/sizeof(void*);
//	vfifo_init(&f, buf, size);
//
//	// up to size-1
//	for(i = 0; i < size; i++)
//	{
//		ASSERT_EQ(vfifo_usage(&f), i);
//		vfifo_put(&f, pointer);
//	}
//	// through overflow
//	for(; i < size+10; i++)
//	{
//		ASSERT_EQ(vfifo_usage(&f), size-1);
//		vfifo_put(&f, pointer);
//	}
//	// back down to 0
//	for(i = size-1; i > 0; i--)
//	{
//		ASSERT_EQ(vfifo_usage(&f), i);
//		vfifo_get(&f, &pointer);
//	}
//	// and through underflow
//	for(i = 10; i > 0; i--)
//	{
//		ASSERT_EQ(vfifo_usage(&f), 0);
//		vfifo_get(&f, &pointer);
//	}
//}
//
//TEST(vfifo_free_tc, starts_at_max_with_non_zero_length)
//{
//	vfifo_t f;
//	void* buf[10] = {NULL};
//	uint8_t size = sizeof(buf)/sizeof(void*);
//	vfifo_init(&f, buf, size);
//	ASSERT_EQ(vfifo_free(&f), size-1);
//}
//
//TEST(vfifo_free_tc, tracks_correctly)
//{
//	uint8_t i;
//	void* pointer = (void*)vfifo_get;
//	vfifo_t f;
//	void* buf[10] = {NULL};
//	uint8_t size = sizeof(buf)/sizeof(void*);
//	vfifo_init(&f, buf, size);
//
//	// up to size-1
//	for(i = 0; i < size; i++)
//	{
//		ASSERT_EQ(vfifo_free(&f), size-1-i);
//		vfifo_put(&f, pointer);
//	}
//	// through overflow
//	for(; i < size+10; i++)
//	{
//		ASSERT_EQ(vfifo_free(&f), 0);
//		vfifo_put(&f, pointer);
//	}
//	// back down to 0
//	for(i = 0; i < size; i++)
//	{
//		ASSERT_EQ(vfifo_free(&f), i);
//		vfifo_get(&f, &pointer);
//	}
//	// and through underflow
//	for(i = 0; i < 10; i++)
//	{
//		ASSERT_EQ(vfifo_free(&f), size-1);
//		vfifo_get(&f, &pointer);
//	}
//}
//
//TEST(fifo_size_tc, size_matches_with_zero_length)
//{
//	void* pointer = (void*)vfifo_get;
//	vfifo_t f;
//	void* buf[10] = {NULL};
//	vfifo_init(&f, buf, 0);
//	ASSERT_EQ(0, vfifo_size(&f));
//}
//
//TEST(fifo_size_tc, size_matches_with_non_zero_length)
//{
//	void* pointer = (void*)vfifo_get;
//	vfifo_t f;
//	void* buf[10] = {NULL};
//	uint8_t size = sizeof(buf)/sizeof(void*);
//	vfifo_init(&f, buf, size);
//	ASSERT_EQ(size-1, vfifo_size(&f));
//}
//
//TEST(fifo_flush_tc, params_are_reset_directly_afterward)
//{
//	void* pointer = (void*)vfifo_get;
//	vfifo_t f;
//	void* buf[10] = {NULL};
//	uint8_t size = sizeof(buf)/sizeof(void*);
//	vfifo_init(&f, buf, size);
//
//	vfifo_put(&f, pointer);
//	vfifo_put(&f, pointer);
//
//	ASSERT_EQ(vfifo_usage(&f), 2);
//	ASSERT_EQ(vfifo_free(&f), vfifo_size(&f)-2);
//	ASSERT_NE(f.head, f.tail);
//	vfifo_flush(&f);
//	ASSERT_EQ(vfifo_usage(&f), 0);
//	ASSERT_EQ(vfifo_free(&f), vfifo_size(&f));
//	ASSERT_EQ(f.head, f.tail);
//}
//
//

