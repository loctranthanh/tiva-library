/*
 * tiva_gps.c
 *
 *  Created on: Dec 3, 2018
 *      Author: toright
 */

#include "tiva_gps.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stdbool.h"
#include "string.h"
#include "tiva_log.h"

typedef struct tiva_gps_ {
    hw_uart_handle_t    uart_handle;
    char*               buffer;
    bool                run;
    gps_t               hgps;
    bool                data_available;
    SemaphoreHandle_t   data_lock;
} tiva_gps_t;

tiva_gps_handle_t tiva_gps_init(tiva_gps_config_t gps_config)
{
    tiva_gps_handle_t gps_handle = calloc(1, sizeof(tiva_gps_t));
    if (gps_handle == NULL) {
        return NULL;
    }
    gps_handle->uart_handle = tiva_uart_init(gps_config.uart_config);
    gps_handle->buffer = (char*)malloc(TIVA_GPS_BUFFER_MAX_SIZE);
    if (gps_handle->buffer == NULL) {
        free(gps_handle->uart_handle);
        free(gps_handle);
        return NULL;
    }
    gps_init(&gps_handle->hgps);
    gps_handle->run = false;
    gps_handle->data_available = false;
    gps_handle->data_lock = xSemaphoreCreateMutex();
    return gps_handle;
}

static void tiva_gps_task(void *pv)
{
    tiva_gps_handle_t gps_handle = (tiva_gps_handle_t)pv;
    uint16_t total_bytes = 0;
    // $GPGGA2352673,232,32323,2,32,32,3,23,23,768
    while (gps_handle->run)
    {
        if (TIVA_GPS_BUFFER_MAX_SIZE - total_bytes <= 0) {
            total_bytes = 0;
        }

        if (!tiva_uart_wait_data(gps_handle->uart_handle, 10 / portTICK_RATE_MS)) {
            continue;
        }

        int read_bytes = tiva_uart_read_bytes(gps_handle->uart_handle, gps_handle->buffer + total_bytes,
                                      TIVA_GPS_BUFFER_MAX_SIZE - total_bytes);
        if (read_bytes <= 0) {
//            vTaskDelay(100 / portTICK_RATE_MS);
            continue;
        }
        total_bytes += read_bytes;
        char *start = memchr(gps_handle->buffer, '$', total_bytes);
        if (start == NULL) {
            total_bytes = 0;
            continue;
        }
//        /* find end of line */
        char *end = memchr(start, '\r', total_bytes - (start - gps_handle->buffer));
        if (NULL == end || '\n' != *(++end)) {
            continue;
        }
//        tiva_logi(start, end - start + 1);

        if (strncmp(start, "$GPGGA", 6) == 0) {
            uint8_t success = 0;
            xSemaphoreTake(gps_handle->data_lock, portMAX_DELAY);
            success = gps_process(&gps_handle->hgps, start, end - start + 1);
            xSemaphoreGive(gps_handle->data_lock);

            /* Print messages */
            if (gps_handle->hgps.latitude != 0 && gps_handle->hgps.longitude != 0 && success == 1) {
                gps_handle->data_available = true;
                int len = sprintf(gps_handle->buffer, "Valid status: %d", gps_handle->hgps.is_valid);
                tiva_logd((char*)gps_handle->buffer, len);
                len = sprintf(gps_handle->buffer, "Latitude: %f degrees", gps_handle->hgps.latitude);
                tiva_logi((char*)gps_handle->buffer, len);
                len = sprintf(gps_handle->buffer, "Longitude: %f degrees", gps_handle->hgps.longitude);
                tiva_logi((char*)gps_handle->buffer, len);
                len = sprintf(gps_handle->buffer, "Time: %d:%d:%d", gps_handle->hgps.hours, gps_handle->hgps.minutes, gps_handle->hgps.seconds);
                tiva_logi((char*)gps_handle->buffer, len);
            }
        }

        if (end == gps_handle->buffer + total_bytes) {
            total_bytes = 0;
            continue;
        }

        if (gps_handle->buffer != memmove(gps_handle->buffer, end, total_bytes - (end - gps_handle->buffer))) {
            total_bytes = 0;
            continue;
        }
        total_bytes -= end - gps_handle->buffer;
    }

    vTaskDelete(NULL);
}

void tiva_gps_run(tiva_gps_handle_t gps_handle)
{
    gps_handle->run = true;
    xTaskCreate(tiva_gps_task, "tiva_gps_task", 512, (void*)gps_handle, 2, NULL);
}

bool tiva_gps_get_data(tiva_gps_handle_t gps_handle, gps_t* gps_data)
{
    if (gps_handle->data_available) {
        xSemaphoreTake(gps_handle->data_lock, portMAX_DELAY);
        memcpy(gps_data, &gps_handle->hgps, sizeof(gps_t));
        xSemaphoreGive(gps_handle->data_lock);
        gps_handle->data_available = false;
        return true;
    }
    return false;
}

void tiva_gps_destroy(tiva_gps_handle_t gps_handle); // TODO: stop task, release buffer


