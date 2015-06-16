
#include "siggen.h"

static void siggen_service_callback(unsigned_stream_type_t* buffer, uint16_t length, uint8_t channels, stream_connection_t* siggen);

void siggen_init(siggen_t* siggen, stream_connection_t* conn, stream_t* stream, const char* name, uint8_t stream_channel, uint16_t* lut, uint32_t length)
{
    log_init(&siggen->log, name);

    stream_connection_init(conn, siggen_service_callback, name, siggen);
    stream_connect_service(conn, stream, stream_channel);

    siggen->lut = lut;
    siggen->lut_length = length;
    siggen->fsa = stream_get_full_scale_amplitude_mv(conn)/2;

    siggen_set_amplitude(conn, SIGGEN_DEFAULT_AMPLITUDE);
    siggen_set_frequency(conn, SIGGEN_DEFAULT_FREQUENCY);
}

void siggen_service_callback(unsigned_stream_type_t* buffer, uint16_t length, uint8_t channels, stream_connection_t* conn)
{
    siggen_t* siggen = (siggen_t*)conn->ctx;
    int i;
    int x;

    for(i = 0; i < length; i++)
    {
        x = (signed_stream_type_t)siggen->lut[siggen->phase_accumulator % siggen->lut_length];
        x *= siggen->amplitude;
        x /= siggen->fsa;// use half of full scale to specify amplitude as peak value
        *buffer = x;
        siggen->phase_accumulator += siggen->tuning_word;
        buffer += channels;
    }
}

/**
 * @brief   sets the frequency of the signal generator.
 * @param   frequency specifies the new frequency in Hz.
 *
 */
void siggen_set_frequency(stream_connection_t* conn, uint32_t frequency)
{
    uint32_t samplerate = stream_get_samplerate(conn->stream);
    siggen_t* siggen = (siggen_t*)conn->ctx;
    siggen->frequency = frequency;

    if(frequency > 0 && frequency < (samplerate/2))
    {
        // reset phase accumulator
        siggen->phase_accumulator = 0;
        // calculate frequency tuning word
        siggen->tuning_word = (unsigned_stream_type_t)((frequency*siggen->lut_length)/samplerate);
        log_info(&siggen->log, "frequency set to %dHz", frequency);
    }
    else
        log_warning(&siggen->log, "frequency %dHz out of range, range is 1 to %dHz", frequency, samplerate/2);
}

/**
 * @retval  returns the signal generator frequency in Hz.
 */
uint32_t siggen_get_frequency(stream_connection_t* conn)
{
    siggen_t* siggen = (siggen_t*)conn->ctx;
    return siggen->frequency;
}

/**
 * @brief   sets the amplitude of the signal generator.
 * @param   amplitude specifies the new amplitude in mV.
 *
 */
void siggen_set_amplitude(stream_connection_t* conn, uint16_t amplitude)
{
    siggen_t* siggen = (siggen_t*)conn->ctx;

    if(amplitude <= siggen->fsa)
    {
        siggen->amplitude = amplitude;
        log_info(&siggen->log, "amplitude set to %dmV", amplitude);
    }
    else
        log_warning(&siggen->log, "amplitude %dmV out of range, range is 0 to %dmV", amplitude, siggen->fsa);
}

/**
 * @retval  returns the signal generator amplitude in mV.
 */
uint16_t siggen_get_amplitude(stream_connection_t* conn)
{
    siggen_t* siggen = (siggen_t*)conn->ctx;
    return siggen->amplitude;
}

void siggen_enable(stream_connection_t* conn, bool enable)
{
    siggen_t* siggen = (siggen_t*)conn->ctx;
    stream_connection_enable(conn, enable);
    log_info(&siggen->log, "siggen %s", conn->enabled ? "enabled" : "disabled");
}

bool siggen_enabled(stream_connection_t* conn)
{
    return conn->enabled;
}

