#include "sim_config.h"
#include <stdio.h>
#include <string.h>

// Default configuration values
static SimConfig currentConfig = {
    .digital_io = {
        .refresh_rate = 100,
        .buffer_size = 1000,
        .pin_count = 16
    },
    .analog_io = {
        .refresh_rate = 1000,
        .buffer_size = 1000,
        .channel_count = 8
    },
    .timing = {
        .default_scale = 1.0f,
        .max_scale = 10.0f
    },
    .monitoring = {
        .log_level = 1,  // INFO
        .buffer_warning_threshold = 80
    }
};

// Helper function to parse key-value pairs from config file
static int parseConfigLine(char* line, char* key, char* value) {
    return sscanf(line, " %[^:]: %s", key, value);
}

void loadSimConfig(const char* configFile) {
    FILE* file = fopen(configFile, "r");
    if (!file) {
        printf("Error: Cannot open config file %s\n", configFile);
        return;
    }

    char line[256];
    char key[64];
    char value[64];

    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }

        if (parseConfigLine(line, key, value) == 2) {
            // Parse digital I/O settings
            if (strcmp(key, "digital_io.refresh_rate") == 0) {
                currentConfig.digital_io.refresh_rate = (uint16_t)atoi(value);
            } else if (strcmp(key, "digital_io.buffer_size") == 0) {
                currentConfig.digital_io.buffer_size = (uint16_t)atoi(value);
            } else if (strcmp(key, "digital_io.pin_count") == 0) {
                currentConfig.digital_io.pin_count = (uint8_t)atoi(value);
            }
            // Parse analog I/O settings
            else if (strcmp(key, "analog_io.refresh_rate") == 0) {
                currentConfig.analog_io.refresh_rate = (uint16_t)atoi(value);
            } else if (strcmp(key, "analog_io.buffer_size") == 0) {
                currentConfig.analog_io.buffer_size = (uint16_t)atoi(value);
            } else if (strcmp(key, "analog_io.channel_count") == 0) {
                currentConfig.analog_io.channel_count = (uint8_t)atoi(value);
            }
            // Parse timing settings
            else if (strcmp(key, "timing.default_scale") == 0) {
                currentConfig.timing.default_scale = (float)atof(value);
            } else if (strcmp(key, "timing.max_scale") == 0) {
                currentConfig.timing.max_scale = (float)atof(value);
            }
            // Parse monitoring settings
            else if (strcmp(key, "monitoring.log_level") == 0) {
                currentConfig.monitoring.log_level = (uint8_t)atoi(value);
            } else if (strcmp(key, "monitoring.buffer_warning_threshold") == 0) {
                currentConfig.monitoring.buffer_warning_threshold = (uint8_t)atoi(value);
            }
        }
    }

    fclose(file);
}

SimConfig* getSimConfig(void) {
    return &currentConfig;
}