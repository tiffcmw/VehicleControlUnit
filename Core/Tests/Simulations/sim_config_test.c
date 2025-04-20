#include "unity.h"
#include "sim_config.h"
#include <stdio.h>

void setUp(void) {
    // Create test config file
    FILE* testConfig = fopen("test_config.txt", "w");
    fprintf(testConfig, "digital_io.refresh_rate: 200\n");
    fprintf(testConfig, "analog_io.channel_count: 16\n");
    fprintf(testConfig, "timing.default_scale: 2.0\n");
    fclose(testConfig);
}

void tearDown(void) {
    remove("test_config.txt");
}

void test_load_config(void) {
    loadSimConfig("test_config.txt");
    SimConfig* config = getSimConfig();
    
    TEST_ASSERT_EQUAL(200, config->digital_io.refresh_rate);
    TEST_ASSERT_EQUAL(16, config->analog_io.channel_count);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, config->timing.default_scale);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_load_config);
    return UNITY_END();
}