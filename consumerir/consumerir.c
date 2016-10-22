/*
 * Copyright (C) 2014 The Android Open Source Project
 * Copyright (C) 2016 The CyanogenMod Project
 * Copyright (C) 2016 Schischu
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define LOG_TAG "ConsumerIrHal"
#define LOG_NDEBUG 0
#include <cutils/log.h>

#include <hardware/hardware.h>
#include <hardware/consumerir.h>

#define FPGA_IR_PATH "/sys/class/sec/sec_ir/ir_send"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define IR_BUFFER_SIZE 4096
#define TMP_BUFFER_SIZE 80

#define UNUSED __attribute__((unused))

static const consumerir_freq_range_t consumerir_freqs[] = {
    {.min = 16000, .max = 60000},
};

static char *SEPARATOR_EOF = "\0";
static char *SEPARATOR_COMMA = ",";

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static int consumerir_transmit_internal(int carrier_freq,
   const int pattern[], int pattern_len)
{
    int i;
    char buffer[IR_BUFFER_SIZE];
    int buffer_len;
    char tmp_buffer[TMP_BUFFER_SIZE];
    char *separator;
    char path[IR_BUFFER_SIZE];
    int fd;
    struct timespec start_time;
    struct timespec current_time;

    snprintf(buffer, IR_BUFFER_SIZE, "%d", carrier_freq);

    for (i = 0; i < pattern_len; i++) {
        if (i == pattern_len - 1)
            separator = SEPARATOR_EOF;
        else
            separator = SEPARATOR_COMMA;

        snprintf(tmp_buffer, TMP_BUFFER_SIZE, "%d%s", pattern[i], separator);

        if (strlcat(buffer, tmp_buffer, IR_BUFFER_SIZE) >= IR_BUFFER_SIZE) {
          ALOGE("Error: pattern is too long\n");
          return -E2BIG;
        }
    }

    strcpy(path, FPGA_IR_PATH);

    clock_gettime(1, &start_time);

    start_time.tv_sec += (start_time.tv_nsec + 100000000) / 1000000000;
    start_time.tv_nsec = (start_time.tv_nsec + 100000000) % 1000000000;

    for (i = 0; i < 10; i++) {
        fd = open(FPGA_IR_PATH, O_RDWR);

        if (fd >= 0)
            break;

        if (errno != EINTR && errno != EACCES) {
            clock_gettime(1, &current_time);
            if (current_time.tv_sec >= start_time.tv_sec &&
                current_time.tv_nsec >= start_time.tv_nsec) {
                ALOGE("Failed to open device. Error: %d", -errno);
                return -errno;
            }
        }
    }

    buffer_len = strlen(buffer);

    if (write(fd, buffer, buffer_len) >= 0) {
        close(fd);
        return 0;
    }

    close(fd);

    return -errno;
}

static int consumerir_transmit(UNUSED struct consumerir_device *dev,
   int carrier_freq, int pattern[], int pattern_len)
{
    int ret;

    pthread_mutex_lock(&g_mutex);

    ret = consumerir_transmit_internal(carrier_freq, pattern, pattern_len);

    if (ret < 0)
        ALOGE("Consumer IR Transmit Failed. Error: %d", ret);

    pthread_mutex_unlock(&g_mutex);

    return ret;
}

static int consumerir_get_num_carrier_freqs(UNUSED struct consumerir_device *dev)
{
    return ARRAY_SIZE(consumerir_freqs);
}

static int consumerir_get_carrier_freqs(UNUSED struct consumerir_device *dev,
    size_t len, consumerir_freq_range_t *ranges)
{
    size_t to_copy = ARRAY_SIZE(consumerir_freqs);

    to_copy = len < to_copy ? len : to_copy;

    memcpy(ranges, consumerir_freqs, to_copy * sizeof(consumerir_freq_range_t));
    return to_copy;
}

static int consumerir_close(hw_device_t *dev)
{
    free(dev);
    return 0;
}

static int consumerir_open(const hw_module_t *module, const char *name,
        hw_device_t **device)
{
    if (strcmp(name, CONSUMERIR_TRANSMITTER) != 0)
        return -EINVAL;

    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

    consumerir_device_t *dev = malloc(sizeof(consumerir_device_t));
    memset(dev, 0, sizeof(consumerir_device_t));

    dev->common.tag            = HARDWARE_DEVICE_TAG;
    dev->common.version        = 0;
    dev->common.module         = (struct hw_module_t*) module;
    dev->common.close          = consumerir_close;

    dev->transmit              = consumerir_transmit;
    dev->get_num_carrier_freqs = consumerir_get_num_carrier_freqs;
    dev->get_carrier_freqs     = consumerir_get_carrier_freqs;

    *device = (hw_device_t*) dev;
    return 0;
}

static struct hw_module_methods_t consumerir_module_methods = {
    .open = consumerir_open,
};

consumerir_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = CONSUMERIR_MODULE_API_VERSION_1_0,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        .id                 = CONSUMERIR_HARDWARE_MODULE_ID,
        .name               = "Consumer IR HAL",
        .author             = "The CyanogenMod Project",
        .methods            = &consumerir_module_methods,
    },
};
