/*
 * tiva_log.h
 *
 *  Created on: Dec 3, 2018
 *      Author: toright
 */

#ifndef _TIVA_LOG_H_
#define _TIVA_LOG_H_


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "string.h"
#include "tiva_uart.h"
#include <stdarg.h>

#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D       LOG_COLOR(LOG_COLOR_CYAN)
#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " " format LOG_RESET_COLOR "\r\n"

typedef struct {
    bool enable_log_info;
    bool enable_log_warning;
    bool enable_log_error;
    bool enable_log_debug;
    hw_uart_config_t uart_config;
} tiva_log_config_t;

void tiva_log_init(tiva_log_config_t log_config);

//bool tiva_logi(char* format, ...);
//
//bool tiva_logd(char* content, uint16_t content_size);
//
//bool tiva_loge(char* format, ...);

bool tiva_log(int level, char* format, ...);

#define TIVA_LOGE(format, ...) tiva_log(0, LOG_FORMAT(E, format), ##__VA_ARGS__)

#define TIVA_LOGI(format, ...) tiva_log(1, LOG_FORMAT(I, format), ##__VA_ARGS__)

#define TIVA_LOGD(format, ...) tiva_log(2, LOG_FORMAT(D, format), ##__VA_ARGS__)

#define TIVA_LOGW(format, ...) tiva_log(3, LOG_FORMAT(W, format), ##__VA_ARGS__)

#endif /* _TIVA_LOG_H_ */
