/**
 * @file log.h
 *
 */

#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdlib.h>  /*exit()*/
#include <stdint.h>  /*uint8_t*/
#include <stdbool.h> /*bool, true, false*/
#include <string.h>  /*strerror()*/
#include <time.h>    /*time(), strftime(), localtime()*/
#include <errno.h>   /*errno*/

#define MICRO_CONTROLLER_LOG

#define TIME_FORMAT "%Y-%m-%d %H:%M:%S" // e.g. Year-Month-Date HH:MM:ss

#ifdef MICRO_CONTROLLER_LOG

#include "myprint.h"

#define logger(fmt, ...)                                    \
    do {                                                    \
        my_print_format(fmt "\n", ##__VA_ARGS__);     \
    } while (0)
#else
#define logger(fmt, ...)                                    \
    do {                                                    \
        time_t now = time(NULL);                            \
        char timestr[20];                                   \
        strftime(timestr, 20, TIME_FORMAT, localtime(&now));\
        printf("%s " fmt "\n", timestr, ##__VA_ARGS__);     \
    } while (0)
#endif

#define error(fmt, ...) logger("ERROR: %s:%d %s(): " fmt ": %s", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__, strerror(errno))
#define warn(fmt, ...)  logger("WARN: %s:%d %s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define info(fmt, ...)  logger("INFO: %s:%d %s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
// #define error(fmt, ...) logger("ERROR: %s() %d: " fmt ": %s", __FUNCTION__, __LINE__, ##__VA_ARGS__, strerror(errno))
// #define warn(fmt, ...)  logger("WARN: %s() %d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
// #define info(fmt, ...)  logger("INFO: %s() %d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define info_mem(p, l)  memdump(p, l);

#ifdef DEBUG
#define debug(fmt, ...) logger("DEBUG: %s:%d %s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define debug_mem(p, l) memdump(p, l);
#else
#define debug(fmt, ...)
#define debug_mem(p, l)
#endif

#define breakpoint()                         \
    do {                                     \
        puts("Press any key to continue...");\
        getchar();                           \
    } while (0)

#endif /*__LOG_H__*/
