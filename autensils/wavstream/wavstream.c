
#include <stdlib.h>
#include "wavstream.h"

#define full_scale_level_mv() ((signed_stream_type_t)stream_get_full_scale_amplitude_mv(conn)/2)

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

    wavstream_set_level(conn, full_scale_level_mv());
}

/**
 * stream callback function.
 */
void wavstream_service_callback_mixdown(unsigned_stream_type_t* buffer, uint16_t length, uint8_t channels, stream_connection_t* conn)
{
    wavstream_t* wavstream = (wavstream_t*)conn->ctx;
    uint32_t samplesread = wav_file_read_mix_to_buffer_channel(&wavstream->file, length, (int16_t*)buffer, channels, 1, wavstream->divider, wavstream->workarea, WAV_STREAM_WORK_AREA_LENGTH);
    if(samplesread < length)
    {
    	wavstream_enable(conn, NULL);
    }
}

/**
 * @brief   sets the level of the signal generator.
 * @param   level specifies the new level in mV.
 *
 */
void wavstream_set_level(stream_connection_t* conn, signed_stream_type_t level)
{
    wavstream_t* wavstream = (wavstream_t*)conn->ctx;

    if(level <= full_scale_level_mv())
    {
        wavstream->divider =  full_scale_level_mv() / level;
        log_info(&wavstream->log, "level set to %dmV", level);
    }
    else
        log_warning(&wavstream->log, "level %dmV out of range, range is 0 to %dmV", level, full_scale_level_mv());
}

/**
 * @retval  returns the signal generator level in mV.
 */
signed_stream_type_t wavstream_get_level(stream_connection_t* conn)
{
    wavstream_t* wavstream = (wavstream_t*)conn->ctx;
    return full_scale_level_mv() / wavstream->divider;
}

/**
 * enables the stream connection to play the specified wave file.
 */
void wavstream_enable(stream_connection_t* conn, const char* file)
{
	bool enable;
    wavstream_t* wavstream = (wavstream_t*)conn->ctx;
    if(file)
    {
		if(!conn->enabled && wav_file_open(&wavstream->file, file) != -1)
		{
			if(((wav_file_get_format(&wavstream->file) == WAVE_FORMAT_PCM) ||
					(wav_file_get_format(&wavstream->file) == WAVE_FORMAT_EXTENSIBLE)) &&
					wav_file_get_channels(&wavstream->file) <= stream_get_channel_count(conn) &&
					wav_file_get_wordsize_bytes(&wavstream->file) <= sizeof(int32_t))
			{
				wavstream->workarea = malloc(WAV_STREAM_WORK_AREA_LENGTH  * wav_file_get_channels(&wavstream->file) * wav_file_get_wordsize_bytes(&wavstream->file));
				enable = wavstream->workarea && wavstream->getsamplerate && wavstream->setsamplerate;
				if(enable)
				{
					wavstream->restore_samplerate = wavstream->getsamplerate();
					wavstream->setsamplerate(wavstream->file.header.fmt_sample_rate);
				}
				else
					log_warning(&wavstream->log, "failed to allocate memory");
				stream_connection_enable(conn, enable);
			}
			else
		        log_warning(&wavstream->log, "cant process format=%d or channelcount=%d or wordsize=%dbits",
		        		wav_file_get_format(&wavstream->file),
						wav_file_get_channels(&wavstream->file),
						wav_file_get_wordsize_bits(&wavstream->file));
		}
    }
    else if(conn->enabled)
    {
	    stream_connection_enable(conn, false);
		wav_file_close(&wavstream->file);
		if(wavstream->setsamplerate)
			wavstream->setsamplerate(wavstream->restore_samplerate);
    	if(wavstream->workarea)
    		free(wavstream->workarea);
    	wavstream->workarea = NULL;
    }
}

/**
 * @retval  returns true if file playback is ongoing.
 */
bool wavstream_enabled(stream_connection_t* conn)
{
    return conn->enabled;
}

