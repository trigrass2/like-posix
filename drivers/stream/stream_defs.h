
#ifndef STREAM_DEFS_H
#define STREAM_DEFS_H

#define STREAM_SR_PERIOD_RELOAD_2K           36000
#define STREAM_SR_PERIOD_RELOAD_4K           18000
#define STREAM_SR_PERIOD_RELOAD_8K           9000
#define STREAM_SR_PERIOD_RELOAD_11K025       6531
#define STREAM_SR_PERIOD_RELOAD_16K          4500
#define STREAM_SR_PERIOD_RELOAD_22K05        3265
#define STREAM_SR_PERIOD_RELOAD_32K          2250
#define STREAM_SR_PERIOD_RELOAD_44K1         1632
#define STREAM_SR_PERIOD_RELOAD_48K          1500
#define STREAM_SR_PERIOD_RELOAD_96K          1250
#define STREAM_SR_MIN                        2000
#define STREAM_SR_MAX                        96000


typedef void(*stream_callback_t)(uint16_t* buffer, uint16_t length, uint8_t channels, uint8_t channel);

typedef struct {
    stream_callback_t process;
    char* name;
    bool enabled;
} stream_connection_t;

typedef struct {
    uint16_t* buffer;                       ///< private to stream driver, points to the current buffer phase or NULL if no data is ready.
    uint16_t* _buffer;                      ///< private to stream driver, points to the start of the full stream buffer.
    stream_connection_t** connections;      ///< private to stream driver, register of service interfaces.
    uint32_t samplerate;                    ///< private to stream driver, holds a cached value of the last set samplerate.
    logger_t log;                           ///< private to stream driver, logger.
    SemaphoreHandle_t ready;                ///< private to stream driver, semaphore used to unblock connection(s) on data ready.
} stream_t;


#endif // STREAM_DEFS_H
