#ifndef SIM_CONFIG_H
#define SIM_CONFIG_H

#include <stdint.h>

typedef struct {
    struct {
        uint16_t refresh_rate;
        uint16_t buffer_size;
        uint8_t pin_count;
    } digital_io;

    struct {
        uint16_t refresh_rate;
        uint16_t buffer_size;
        uint8_t channel_count;
    } analog_io;

    struct {
        float default_scale;
        float max_scale;
    } timing;

    struct {
        uint8_t log_level;
        uint8_t buffer_warning_threshold;
    } monitoring;
} SimConfig;

void loadSimConfig(const char* configFile);
SimConfig* getSimConfig(void);

#endif /* SIM_CONFIG_H */