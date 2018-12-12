/*
 * tiva_uart.h
 *
 *  Created on: Dec 1, 2018
 *      Author: toright
 */

#ifndef _TIVA_UART_H_
#define _TIVA_UART_H_


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define HW_UART_FRAME_CONFIG_DEFAULT    (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE)
#define DEFAULT_MAX_BUFFER_SIZE         512

typedef enum {
    HW_UART_NUM_0,
    HW_UART_NUM_1,
    HW_UART_NUM_2,
    HW_UART_NUM_3,
    HW_UART_NUM_4,
    HW_UART_NUM_5,
} hw_uart_num_t;

typedef struct {
    hw_uart_num_t   hw_uart_num;
    uint32_t        baudrate;
    uint32_t        frame_config;
    uint16_t        max_buffer_size;
} hw_uart_config_t;

typedef struct hw_uart_* hw_uart_handle_t;

hw_uart_handle_t tiva_uart_init(hw_uart_config_t config);
void tiva_uart_write_bytes(hw_uart_handle_t uart_handle, uint8_t* buffer, uint16_t buffer_len);
void tiva_uart_write_string(hw_uart_handle_t uart_handle, char* s_data);
uint16_t tiva_uart_read_bytes(hw_uart_handle_t uart_handle, uint8_t* buffer, uint16_t buffer_len);
bool tiva_uart_data_available(hw_uart_handle_t uart_handle);
char tiva_uart_read_char(hw_uart_handle_t uart_handle);
bool tiva_uart_wait_data(hw_uart_handle_t uart_handle, uint32_t tick);

#endif /* _TIVA_UART_H_ */
