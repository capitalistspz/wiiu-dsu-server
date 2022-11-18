/*
 * Taken from https://github.com/wiiu-env/WiiUPluginSystem/blob/master/plugins/example_plugin/src/utils/logger.h
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <whb/log.h>
#include <whb/crash.h>

#define __FILENAME_X__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __FILENAME__   (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILENAME_X__)

#define OSFATAL_FUNCTION_LINE(FMT, ARGS...)                                                    \
    do {                                                                                       \
        OSFatal_printf("[%s]%s@L%04d: " FMT "", __FILENAME__, __FUNCTION__, __LINE__, ##ARGS); \
    } while (0)

#define DEBUG_FUNCTION_LINE(FMT, ARGS...)                                                        \
    do {                                                                                         \
        WHBLogPrintf("[%s][%23s]%30s@L%04d: " FMT "", APPLICATION_NAME, __FILENAME__, __FUNCTION__, __LINE__, ##ARGS); \
    } while (0);

#define DEBUG_FUNCTION_LINE_WRITE(FMT, ARGS...)                                                  \
    do {                                                                                         \
        WHBLogWritef("[%23s]%30s@L%04d: " FMT "", __FILENAME__, __FUNCTION__, __LINE__, ##ARGS); \
    } while (0);

#ifdef __cplusplus
}
#endif
