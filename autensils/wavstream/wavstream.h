

#ifndef WAVSTREAM_H_
#define WAVSTREAM_H_

#include "wav.h"
#include "stream_common.h"

#define WAV_STREAM_WORK_AREA_LENGTH 256

typedef struct {
    wav_file_t file;
    signed_stream_type_t divider;
    logger_t log;
    int32_t fsa;
    uint32_t restore_samplerate;
    uint32_t(*getsamplerate)(void);
    void(*setsamplerate)(uint32_t);
    signed_stream_type_t* workarea;
} wavstream_t;

void wavstream_init(wavstream_t* wavstream, stream_connection_t* conn, stream_t* stream, const char* name, int8_t stream_channel, uint32_t(*getsamplerate)(void), void(*setsamplerate)(uint32_t));
void wavstream_enable(stream_connection_t* conn, const char* file);
bool wavstream_enabled(stream_connection_t* conn);
void wavstream_set_level(stream_connection_t* conn, signed_stream_type_t level);
signed_stream_type_t wavstream_get_level(stream_connection_t* conn);

#endif /* WAVSTREAM_H_ */




