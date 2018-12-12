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

//#define LOG_COLOR_GREEN   "32"
//#define LOG_RESET_COLOR   "\033[0m"
//#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
//#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
//#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " " format LOG_RESET_COLOR "\n"

typedef struct {
    bool enable_log_info;
    bool enable_log_warning;
    bool enable_log_error;
    bool enable_log_debug;
    hw_uart_config_t uart_config;
} tiva_log_config_t;

void tiva_log_init(tiva_log_config_t log_config);

bool tiva_logi(char* content, uint16_t content_size);

bool tiva_logd(char* content, uint16_t content_size);

//#define TIVA_LOGE(format, ...) tiva_loge(LOG_FORMAT(I, format), ##__VA_ARGS__)

#endif /* _TIVA_LOG_H_ */
