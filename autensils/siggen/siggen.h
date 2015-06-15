

#ifndef SIGGEN_H_
#define SIGGEN_H_

#include "stream_common.h"


extern const uint16_t sine_unsigned_65536[65536];
extern const int16_t sine_signed_32768[32768];
extern const uint16_t sine_unsigned_32768[32768];
extern const uint16_t sine_unsigned_16384[16384];
extern const uint16_t sine_unsigned_8192[8192];
extern const uint16_t sine_unsigned_4096[4096];
extern const uint16_t sine_unsigned_2048[2048];
extern const uint16_t sine_unsigned_1024[1024];
extern const uint16_t sine_unsigned_512[512];

#define SIGGEN_DEFAULT_FREQUENCY           1000
#define SIGGEN_DEFAULT_AMPLITUDE           100

typedef struct {
    uint16_t phase_accumulator;
    uint16_t tuning_word;
    const uint16_t* lut;
    uint32_t lut_length;
    uint32_t frequency;
    uint16_t amplitude;
    logger_t log;
    int32_t fsa;
} siggen_t;

void siggen_init(siggen_t* siggen, stream_connection_t* conn, stream_t* stream, const char* name, uint8_t stream_channel, uint16_t* lut, uint32_t length);
void siggen_set_frequency(stream_connection_t* conn, uint32_t frequency);
uint32_t siggen_get_frequency(stream_connection_t* conn);
void siggen_enable(stream_connection_t* conn, bool enable);
bool siggen_enabled(stream_connection_t* conn);
void siggen_set_amplitude(stream_connection_t* conn, uint16_t amplitude);
uint16_t siggen_get_amplitude(stream_connection_t* conn);

#endif /* SIGGEN_H_ */




