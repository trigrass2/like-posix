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
 * This file is part of the like-posix project, <https://github.com/drmetal/like-posix>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include "wavstream.h"


static void wavstream_service_callback_mixdown(unsigned_stream_type_t* buffer, uint16_t length, uint8_t channels, stream_connection_t* wavstream);

/**
 * streams a wave file out of the specified stream device.
 *
 * **NOTE**
 * - this module will only work with a stream that supports signed data (i2s_stream is an example)
 * - supports 8b unsigned PCM, 16bit signed PCM, 32bit signed PCM (extensible format not validated)
 *
 * @param wavstream is the wavstream stucture to initialize.
 * @param conn is the stream connection to initialize.
 * @param stream is the stream to connect to (must already be initialized).
 * @param name is the name of this module, can be "wavstream" for example.
 * @param stream_channel the channel in the stream, to operate from.0, 1 ... etc if the wave stream only needs to operate on one channel at a time.
 * 			@todo - write a version where -1 specifies  play wave files to stereo stream no matter if file is mono or stereo.
 */
void wavstream_init(wavstream_t* wavstream, stream_connection_t* conn, stream_t* stream, const char* name, int8_t stream_channel, uint32_t(*getsamplerate)(void), void(*setsamplerate)(uint32_t))
{
    log_init(&wavstream->log, name);

    wavstream->getsamplerate = getsamplerate;
    wavstream->setsamplerate = setsamplerate;

	stream_connection_init(conn, wavstream_service_callback_mixdown, name, wavstream);
	stream_connect_service(conn, stream, stream_channel);

	wavstream->fsa = stream_get_full_scale_amplitude_mv(conn)/2;

	memset(&wavstream->wavproc, 0, sizeof(wav_file_processing_t));

	init_wavstream_mutex();

    wavstream_set_level(conn, wavstream->fsa);
}

/**
 * stream callback function.
 *
 * operates on one stream channel only.
 */
void wavstream_service_callback_mixdown(unsigned_stream_type_t* buffer, uint16_t length, uint8_t channels, stream_connection_t* conn)
{
	uint32_t samplesread = 0;
	uint16_t i;
    wavstream_t* wavstream = (wavstream_t*)conn->ctx;

	if(wavstream_take_mutex() && conn->enabled)
	{
		// wave file part
		wav_file_buffer_setup(&wavstream->wavproc, buffer, length, channels);

		samplesread = wav_file_read_mix_to_buffer_channel(&wavstream->file, &wavstream->wavproc);

		// scale to user defined level
		for(i = 0; i < length; i++)
		{
			*buffer = *((signed_stream_type_t*)buffer) * wavstream->level / wavstream->fsa;
			buffer += channels;
		}
		wavstream_give_mutex();
	}

    if(samplesread < length)
    	wavstream_enable(conn, NULL);
}

/**
 * @brief   sets the level of the signal generator.
 * @param   level specifies the new level in mV.
 *
 */
void wavstream_set_level(stream_connection_t* conn, signed_stream_type_t level)
{
    wavstream_t* wavstream = (wavstream_t*)conn->ctx;

    if(level <= wavstream->fsa)
    {
        wavstream->level =  level;
        log_info(&wavstream->log, "level set to %dmV", level);
    }
    else
        log_warning(&wavstream->log, "level %dmV out of range, range is 0 to %dmV", level, wavstream->fsa);
}

/**
 * @retval  returns the signal generator level in mV.
 */
signed_stream_type_t wavstream_get_level(stream_connection_t* conn)
{
    wavstream_t* wavstream = (wavstream_t*)conn->ctx;
    return wavstream->level;
}

/**
 * enables the stream connection to play the specified wave file.
 * if file is set to NULL, stops any ongoing playback.
 */
void wavstream_enable(stream_connection_t* conn, const char* file)
{
	bool enable;
    wavstream_t* wavstream = (wavstream_t*)conn->ctx;

    assert_true(wavstream->getsamplerate && wavstream->setsamplerate);

	if(wavstream_take_mutex())
	{
		if(file)
		{
			log_debug(&wavstream->log, "open file %s", file);
			if(!conn->enabled && wav_file_open(&wavstream->file, file) != -1)
			{
				if(wav_file_get_format(&wavstream->file) != WAVE_FORMAT_PCM &&
				   wav_file_get_format(&wavstream->file) != WAVE_FORMAT_EXTENSIBLE)
				{
					log_warning(&wavstream->log, "wave file must be PCM (%d) or Extensible format (%d), it is %d",
													WAVE_FORMAT_PCM, WAVE_FORMAT_EXTENSIBLE, wav_file_get_format(&wavstream->file));
				}
				else if(wav_file_get_channels(&wavstream->file) > stream_get_channel_count(conn))
				{
					log_warning(&wavstream->log, "number of channels must be less than %d, it was %d",
													stream_get_channel_count(conn), wav_file_get_channels(&wavstream->file));
				}
				else if(wav_file_get_wordsize_bytes(&wavstream->file) > sizeof(int32_t))
				{
					log_warning(&wavstream->log, "word length must be less than %db, it was %db",
													sizeof(int32_t)*8, wav_file_get_wordsize_bits(&wavstream->file));
				}
				else
				{
					enable = wav_file_init_stream_params(&wavstream->file, &wavstream->wavproc,
														sizeof(signed_stream_type_t), WAV_FILE_WORK_AREA_LENGTH);
					if(enable)
					{
						// set the stream samplerate to match the file
						wavstream->restore_samplerate = wavstream->getsamplerate();
						if(wavstream->restore_samplerate != wavstream->file.header.fmt_sample_rate)
							wavstream->setsamplerate(wavstream->file.header.fmt_sample_rate);

						log_debug(&wavstream->log, "data length: %u", wav_file_get_data_length(&wavstream->file));
						log_debug(&wavstream->log, "channels: %u", wav_file_get_channels(&wavstream->file));
						log_debug(&wavstream->log, "samplerate: %u", wav_file_get_samplerate(&wavstream->file));
						log_debug(&wavstream->log, "resolution: %u", wav_file_get_wordsize_bits(&wavstream->file));
						stream_connection_enable(conn, true);
					}
					else
						log_warning(&wavstream->log, "failed to initialize work area");
				}
			}
		}
		else if(conn->enabled)
		{
			stream_connection_enable(conn, false);
			wav_file_close(&wavstream->file);
			if(wavstream->restore_samplerate != wavstream->getsamplerate())
				wavstream->setsamplerate(wavstream->restore_samplerate);
			wav_file_deinit_stream_params(&wavstream->wavproc);
		}

		wavstream_give_mutex();
    }

    log_info(&wavstream->log, "playback %s", conn->enabled ? "started" : "stopped");
}

/**
 * @retval  returns true if file playback is ongoing.
 */
bool wavstream_enabled(stream_connection_t* conn)
{
    return conn->enabled;
}

