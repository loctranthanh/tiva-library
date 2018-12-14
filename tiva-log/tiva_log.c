/*
 * tiva_log.c
 *
 *  Created on: Dec 3, 2018
 *      Author: toright
 */

#include "tiva_log.h"
#include <stdarg.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "stdio.h"

#define TIVA_LOG_MAX_BUFFER_SIZE    128

typedef struct tiva_log_ {
    hw_uart_handle_t uart_handle;
    bool enable_log_info;
    bool enable_log_warning;
    bool enable_log_error;
    bool enable_log_debug;
    SemaphoreHandle_t mutext_lock;
    char* buffer;
} tiva_log_t;

typedef struct tiva_log_* tiva_log_handle_t;

tiva_log_handle_t g_log_handle = NULL;

void tiva_log_init(tiva_log_config_t log_config)
{
    g_log_handle = calloc(1, sizeof(tiva_log_t));
    if (g_log_handle == NULL) {
        return;
    }
    g_log_handle->buffer = (char*)malloc(TIVA_LOG_MAX_BUFFER_SIZE);
    if (g_log_handle->buffer == NULL) {
        free(g_log_handle);
        g_log_handle = NULL;
        return;
    }
    g_log_handle->enable_log_error = log_config.enable_log_error;
    g_log_handle->enable_log_info = log_config.enable_log_info;
    g_log_handle->enable_log_warning = log_config.enable_log_warning;
    g_log_handle->enable_log_debug = log_config.enable_log_debug;
    g_log_handle->uart_handle = tiva_uart_init(log_config.uart_config);
    if (g_log_handle->uart_handle == NULL) {
        free(g_log_handle->buffer);
        free(g_log_handle);
        g_log_handle = NULL;
        return;
    }
    g_log_handle->mutext_lock = xSemaphoreCreateMutex();
}

//bool tiva_logi(char* content, uint16_t content_size)
//{
//    if (g_log_handle == NULL || g_log_handle->enable_log_info == false || content == NULL) {
//        return false;
//    }
//    xSemaphoreTake(g_log_handle->mutext_lock, portMAX_DELAY);
//    tiva_uart_write_bytes(g_log_handle->uart_handle, "[INFO] ", 7);
//    tiva_uart_write_bytes(g_log_handle->uart_handle, (uint8_t*)content, content_size);
//    tiva_uart_write_bytes(g_log_handle->uart_handle, "\r\n", 2);
//    xSemaphoreGive(g_log_handle->mutext_lock);
//    return true;
//}
//
//bool tiva_logd(char* content, uint16_t content_size)
//{
//    if (g_log_handle == NULL || g_log_handle->enable_log_debug == false || content == NULL) {
//        return false;
//    }
//    xSemaphoreTake(g_log_handle->mutext_lock, portMAX_DELAY);
//    tiva_uart_write_bytes(g_log_handle->uart_handle, "[DEBUG] ", 8);
//    tiva_uart_write_bytes(g_log_handle->uart_handle, (uint8_t*)content, content_size);
//    tiva_uart_write_bytes(g_log_handle->uart_handle, "\r\n", 2);
//    xSemaphoreGive(g_log_handle->mutext_lock);
//    return true;
//}
//
//bool tiva_loge(char* content, ...)
//{
//    if (g_log_handle == NULL || g_log_handle->enable_log_error == false || content == NULL) {
//        return false;
//    }
//    xSemaphoreTake(g_log_handle->mutext_lock, portMAX_DELAY);
//    va_list list;
//    va_start(list, content);
//    int len = vsprintf(g_log_handle->buffer, content, list);
//    va_end(list);
//    tiva_uart_write_bytes(g_log_handle->uart_handle, (uint8_t*)g_log_handle->buffer, len);
//    xSemaphoreGive(g_log_handle->mutext_lock);
//    return true;
//}

bool tiva_log(int level, char* format, ...)
{
    if (g_log_handle == NULL || format == NULL) {
        return false;
    }
    if (level == 0 && g_log_handle->enable_log_error == false) {
        return false;
    } else if (level == 1 && g_log_handle->enable_log_info == false) {
        return false;
    } else if (level == 2 && g_log_handle->enable_log_debug == false) {
        return false;
    } else if (level == 3 && g_log_handle->enable_log_warning == false) {
        return false;
    } else if (level > 3) {
        return false;
    }
    xSemaphoreTake(g_log_handle->mutext_lock, portMAX_DELAY);
    va_list list;
    va_start(list, format);
    int len = vsprintf(g_log_handle->buffer, format, list);
    va_end(list);
    tiva_uart_write_bytes(g_log_handle->uart_handle, (uint8_t*)g_log_handle->buffer, len);
    xSemaphoreGive(g_log_handle->mutext_lock);
    return true;
}
