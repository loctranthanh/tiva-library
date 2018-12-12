/*
 * tiva_gps.h
 *
 *  Created on: Dec 3, 2018
 *      Author: toright
 */

#ifndef _TIVA_GPS_H_
#define _TIVA_GPS_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "tiva_uart.h"
#include "gps/gps.h"
#include "gps/gps_buff.h"

#define TIVA_GPS_BUFFER_MAX_SIZE    512

typedef struct {
    hw_uart_config_t uart_config;
} tiva_gps_config_t;

typedef struct tiva_gps_* tiva_gps_handle_t;

tiva_gps_handle_t tiva_gps_init(tiva_gps_config_t gps_config);

void tiva_gps_run(tiva_gps_handle_t gps_handle);

bool tiva_gps_get_data(tiva_gps_handle_t gps_handle, gps_t* gps_data);

void tiva_gps_destroy(tiva_gps_handle_t gps_handle);

#endif /* _TIVA_GPS_H_ */
