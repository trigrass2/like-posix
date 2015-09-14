/*
 * Copyright (c) 2015 Michael Stuart.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the Appleseed project, <https://github.com/drmetal/app-l-seed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "wav.h"


const uint8_t RIFF[] = RIFF_TAG_RIFF;
const uint8_t WAVE[] = RIFF_TAG_WAVE;
const uint8_t FMT[] = RIFF_TAG_FMT ;
const uint8_t DATA[] = RIFF_TAG_DATA;
const uint8_t FACT[] = RIFF_TAG_FACT;


//static const riff_header_t riffWavTemplate = WAVE_HEADER_TEMPLATE;


/**
 * read one chunk tag of CHUNK_TAG_LENGTH bytes in length from current position in the file, into the memory pointed to by field.
 */
static bool wav_file_read_chunk(wav_file_t* file, void* field)
{
	return read(file->fdes, field, CHUNK_TAG_LENGTH) == CHUNK_TAG_LENGTH;
}

/**
 * compare chunk tag of CHUNK_TAG_LENGTH bytes to another. return true if they match, false otherwise.
 */
static bool wav_file_compare_chunk(const void* field, const void* compare)
{
	return memcmp(field, compare, CHUNK_TAG_LENGTH) == 0;
}

/**
 * read a short integer 2 bytes in length from the current position in the file, into the integer pointed to by value.
 */
static bool wav_file_read_short(wav_file_t* file, uint16_t* value)
{
	return read(file->fdes, value, sizeof(uint16_t)) == sizeof(uint16_t);
}

/**
 * read a long integer 4 bytes in length from the current position in the file, into the integer pointed to by value.
 */
static bool wav_file_read_integer(wav_file_t* file, uint32_t* value)
{
	return read(file->fdes, value, sizeof(uint32_t)) == sizeof(uint32_t);
}

/**
 * read an entire wave file header into the given wav_file_t file structure. return false if an error occurred while reading.
 */
bool wav_file_unpack_header(wav_file_t* file)
{
//	log_debug(NULL, "%d", file->fdes);
	// read RIFF chunk
	if(!wav_file_read_chunk(file, file->header.riff) || !wav_file_compare_chunk(file->header.riff, RIFF))
		return false;

	// read SIZE field
	if(!wav_file_read_integer(file, &file->header.size))
		return false;

	// read WAVE chunk
	if(!wav_file_read_chunk(file, file->header.type) || !wav_file_compare_chunk(file->header.type, WAVE))
		return false;

	// read FMT chunk and all fmt data
	if(!wav_file_read_chunk(file, file->header.fmt) || !wav_file_compare_chunk(file->header.fmt, FMT))
		return false;

	wav_file_read_integer(file, &file->header.fmt_len);
	wav_file_read_short(file, &file->header.fmt_tag);
	wav_file_read_short(file, &file->header.fmt_num_channels);
	wav_file_read_integer(file, &file->header.fmt_sample_rate);
	wav_file_read_integer(file, &file->header.fmt_byte_rate);
	wav_file_read_short(file, &file->header.fmt_block_align);
	wav_file_read_short(file, &file->header.fmt_word_size);

	// read next chunk (FACT or DATA) tag
	if(!wav_file_read_chunk(file, file->header.data))
		return false;

	// it is the data chunk tag
	if(wav_file_compare_chunk(file->header.data, DATA))
	{
		// populate fact chunk with dummy data
		memcpy(file->header.fact, FACT, CHUNK_TAG_LENGTH);
		file->header.fact_len = 0;
	}
	// it is the fact chunk tag
	else if(wav_file_compare_chunk(file->header.data, FACT))
	{
		memcpy(file->header.fact, file->header.data, CHUNK_TAG_LENGTH);
		wav_file_read_integer(file, &file->header.fact_len);
		// seek up to next chunk (ignore fact chunk)
		lseek(file->fdes, file->header.fact_len, SEEK_CUR);
		// now read the data chunk tag
		wav_file_read_chunk(file, file->header.data);
	}

	wav_file_read_integer(file, &file->header.data_len);
	return true;
}

/**
 * open an existing wave file for reading, from filepath.
 * reads the header, returning 0 on success, -1 on failure.
 */
int wav_file_open(wav_file_t* file, const char* filepath)
{
	file->fdes = open(filepath, O_RDONLY);

	if(!wav_file_unpack_header(file))
	{
		close(file->fdes);
		file->fdes = -1;
	}
	return file->fdes;
}

/**
 * closes the wave file.
 */
void wav_file_close(wav_file_t* file)
{
	close(file->fdes);
}

/**
 * populates the given wav_file_processing_t processing structure transformation parameters.
 *
 * called by a stream service once when enabling the stream, after the wave file has been opened.
 *
 * Note: allocates a working area buffer, so the function wav_file_deinit_stream_params() must be called when the wave file is finished playing.
 */
bool wav_file_init_stream_params(wav_file_t* file, wav_file_processing_t* wavproc, uint8_t buffer_width, uint16_t workarealength)
{
	wavproc->wav_word_size_bytes = file->header.fmt_word_size / 8;
	wavproc->workarea_length_samples = workarealength * file->header.fmt_num_channels;
	wavproc->wav_read_length_bytes = wavproc->workarea_length_samples * wavproc->wav_word_size_bytes;
	if(wavproc->workarea == NULL)
		wavproc->workarea = malloc(wavproc->wav_read_length_bytes);
	wavproc->buffer_word_size_bytes = buffer_width;

	return wavproc->workarea != NULL;
}

/**
 * cleans up after wav_file_init_stream_params() has been called.
 */
void wav_file_deinit_stream_params(wav_file_processing_t* wavproc)
{
	if(wavproc->workarea)
		free(wavproc->workarea);
	wavproc->workarea = NULL;
}

/**
 * populates the given wav_file_processing_t processing structure buffer parameters.
 *
 * called by a stream service before calling wav_file_read_mix_to_buffer_channel().
 * may be called multiple times.
 */
void wav_file_buffer_setup(wav_file_processing_t* wavproc, void* buffer, uint32_t buffer_length_samples, uint8_t buffer_channels)
{
	wavproc->buffer_channels = buffer_channels;
	wavproc->buffer = buffer;
	wavproc->buffer_length_samples = buffer_length_samples;
}

/**
 * read all channels from N wave file, mixing signal down into one stream buffer channel.
 *
 * supports 8, 16 and 32 bit wave files, of 1 or more channels.
 * does not perform re-sampling.
 * does not scale the data.
 *
 * @param file is the open wave file
 * @param wavproc is an initialized wav_file_processing_t structure. initialize with wav_file_init_stream_params() and wav_file_buffer_setup().
 * @param multiply is a number to multiply samples by before after summing. it can be used to amplify the output data.
 */
uint32_t wav_file_read_mix_to_buffer_channel(wav_file_t* file, wav_file_processing_t* wavproc)
{
	uint32_t wordsread = wavproc->workarea_length_samples;
	uint32_t samplecount = 0;
	void* scratch;
	void* buffer = wavproc->buffer;
	uint32_t word;
	uint16_t i;
	int32_t sum;

	// one loop per 256 samples
	while(wordsread == wavproc->workarea_length_samples && samplecount < wavproc->buffer_length_samples)
	{
		wordsread = read(file->fdes, wavproc->workarea, wavproc->wav_read_length_bytes);
		wordsread /= wavproc->wav_word_size_bytes;
		scratch = wavproc->workarea;

		// one loop per sample in wave file
		for(word = 0; word < wordsread; word += file->header.fmt_num_channels)
		{
			sum = 0;
			// one loop per channel in the sample
			switch(wavproc->wav_word_size_bytes)
			{
				case sizeof(uint8_t):
					for(i = 0; i < file->header.fmt_num_channels; i++)
					{
						sum += (*(uint8_t*)scratch)-128; // 8bit wave is unsigned
						scratch = ((uint8_t*)scratch) + wavproc->wav_word_size_bytes;
					}
					sum *= 256; // normalize to 16bits
				break;
				case sizeof(int16_t):
					for(i = 0; i < file->header.fmt_num_channels; i++)
					{
						sum += *(int16_t*)scratch;
						scratch = ((uint8_t*)scratch) + wavproc->wav_word_size_bytes;
					}
				break;
				case sizeof(int32_t):
					for(i = 0; i < file->header.fmt_num_channels; i++)
					{
						sum += (*(int32_t*)scratch)/65536; // avoid overflow, divide every cycle, normalize to 16bits
						scratch = ((uint8_t*)scratch) + wavproc->wav_word_size_bytes;
					}
				break;

			}

			switch(wavproc->buffer_word_size_bytes)
			{
				case sizeof(int8_t):
					*(int8_t*)buffer = sum / file->header.fmt_num_channels;
				break;
				case sizeof(int16_t):
					*(int16_t*)buffer = sum / file->header.fmt_num_channels;
				break;
				case sizeof(int32_t):
					*(int32_t*)buffer = sum / file->header.fmt_num_channels;
				break;
			}

			buffer = ((uint8_t*)buffer) + (wavproc->buffer_channels * wavproc->buffer_word_size_bytes);
			samplecount++;
		}
	}

	return samplecount;
}

/**
 * returns the number of channels in the wave file.
 * note: the wave file needs to have been successfully opened first.
 */
uint16_t wav_file_get_channels(wav_file_t* file)
{
	return file->header.fmt_num_channels;
}

/**
 * returns the number of bytes in the wave data field.
 * note: the wave file needs to have been successfully opened first.
 */
uint32_t wav_file_get_data_length(wav_file_t* file)
{
	return file->header.data_len;
}

/**
 * returns the samplerate in Hz of the wave file.
 * note: the wave file needs to have been successfully opened first.
 */
uint32_t wav_file_get_samplerate(wav_file_t* file)
{
	return file->header.fmt_sample_rate;
}

/**
 * returns the size of the samples, in bits.
 * note: the wave file needs to have been successfully opened first.
 */
uint32_t wav_file_get_wordsize_bits(wav_file_t* file)
{
	return file->header.fmt_word_size;
}

/**
 * returns the size of the samples, in bytes.
 * note: the wave file needs to have been successfully opened first.
 */
uint32_t wav_file_get_wordsize_bytes(wav_file_t* file)
{
	return file->header.fmt_word_size/8;
}

/**
 * returns the format of the file.
 * note: the wave file needs to have been successfully opened first.
 */
wave_fmt_t wav_file_get_format(wav_file_t* file)
{
	return file->header.fmt_tag;
}


