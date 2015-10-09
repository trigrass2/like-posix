
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "gtest/gtest.h"
#include "wav.h"


#define EXPECTED_DATA_LENGTH 220160

TEST(test_wav, wav_file_open_close_fails)
{
	wav_file_t f;
	int ret = wav_file_open(&f, "none.wav");
	wav_file_close(&f);
    ASSERT_EQ(ret, -1);
}

TEST(test_wav, wav_file_open_close_passes)
{
	wav_file_t f;
	int ret;
	ret = wav_file_open(&f, "test.wav");

	ASSERT_EQ(f.header.data_len, EXPECTED_DATA_LENGTH);
	ASSERT_EQ(memcmp(f.header.fact, (uint8_t*)"fact", 4), 0);
	ASSERT_EQ(f.header.fact_len, 0);
	ASSERT_EQ(memcmp(f.header.fmt, (uint8_t*)"fmt ", 4), 0);
	ASSERT_EQ(f.header.fmt_block_align, 2);
	ASSERT_EQ(f.header.fmt_byte_rate, 22050);
	ASSERT_EQ(f.header.fmt_len, 16);
	ASSERT_EQ(f.header.fmt_num_channels, 1);
	ASSERT_EQ(f.header.fmt_sample_rate, 11025);
	ASSERT_EQ(f.header.fmt_tag, 1);
	ASSERT_EQ(f.header.fmt_word_size, 16);
	ASSERT_EQ(memcmp(f.header.riff, (uint8_t*)"RIFF", 4), 0);
	ASSERT_EQ(f.header.size, 220196);
	ASSERT_EQ(memcmp(f.header.type, (uint8_t*)"WAVE", 4), 0);

	wav_file_close(&f);
    ASSERT_NE(ret, -1);
}

TEST(test_wav, wav_file_init_stream_params)
{
	uint32_t l = 1;
	uint32_t length;
	uint16_t wavreadlength = 256;
	uint16_t samples = 512;
	uint16_t channels = 2;
	uint16_t buffer[samples * channels];
	uint16_t buffer_channel = 0;

	wav_file_processing_t wp;
	wav_file_t f;
	int ret;

	memset(&wp, 0, sizeof(wav_file_processing_t));

	ASSERT_EQ(wp.workarea, (void*)NULL);

	ret = wav_file_open(&f, "test.wav");
	bool init = wav_file_init_stream_params(&f, &wp, 2, wavreadlength);

	wav_file_buffer_setup(&wp, &buffer[buffer_channel], samples, channels);

	ASSERT_EQ(wp.buffer, (void*)&buffer[buffer_channel]);
	ASSERT_EQ(wp.buffer_channels, channels);
	ASSERT_EQ(wp.buffer_length_samples, samples);
	ASSERT_EQ(wp.buffer_word_size_bytes, sizeof(uint16_t));
	ASSERT_EQ(wp.wav_read_length_bytes, wavreadlength*f.header.fmt_num_channels*f.header.fmt_word_size/8);
	ASSERT_EQ(wp.wav_word_size_bytes, f.header.fmt_word_size/8);
	ASSERT_NE(wp.workarea, (void*)NULL);
	ASSERT_EQ(wp.workarea_length_samples, wavreadlength);

	length = 0;
	while(l > 0)
	{
		l = wav_file_read_mix_to_buffer_channel(&f, &wp);
		length += l;
	}

	ASSERT_EQ(length, EXPECTED_DATA_LENGTH/(f.header.fmt_word_size/8));

	wav_file_deinit_stream_params(&wp);
	wav_file_close(&f);

    ASSERT_TRUE(init);
    ASSERT_NE(ret, -1);
}
