/*
 * tiva_uart.c
 *
 *  Created on: Dec 1, 2018
 *      Author: toright
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "tiva_uart.h"
#include "tiva_periph.h"
#include "string.h"
#include "FreeRTOS.h"
#include "semphr.h"

#define HW_UART0_INFO   {   .sys_periph_uart = SYSCTL_PERIPH_UART0, .sys_periph_gpio = SYSCTL_PERIPH_GPIOA, \
                            .pin_config_tx = GPIO_PA1_U0TX,  .pin_config_rx = GPIO_PA0_U0RX,                \
                            .gpio_pins = GPIO_PIN_0 | GPIO_PIN_1, .gpio_port_base = GPIO_PORTA_BASE,        \
                            .uart_base = UART0_BASE, .uart_int = INT_UART0                                  \
                        }
#define HW_UART1_INFO   {   .sys_periph_uart = SYSCTL_PERIPH_UART1, .sys_periph_gpio = SYSCTL_PERIPH_GPIOB, \
                            .pin_config_tx = GPIO_PB1_U1TX,  .pin_config_rx = GPIO_PB0_U1RX,                \
                            .gpio_pins = GPIO_PIN_0 | GPIO_PIN_1, .gpio_port_base = GPIO_PORTB_BASE,        \
                            .uart_base = UART1_BASE, .uart_int = INT_UART1                                  \
                        }
#define HW_UART2_INFO   {   .sys_periph_uart = SYSCTL_PERIPH_UART2, .sys_periph_gpio = SYSCTL_PERIPH_GPIOD, \
                            .pin_config_tx = GPIO_PD7_U2TX,  .pin_config_rx = GPIO_PD6_U2RX,                \
                            .gpio_pins = GPIO_PIN_6 | GPIO_PIN_7, .gpio_port_base = GPIO_PORTD_BASE,        \
                            .uart_base = UART2_BASE, .uart_int = INT_UART2                                  \
                        }
#define HW_UART3_INFO   {   .sys_periph_uart = SYSCTL_PERIPH_UART3, .sys_periph_gpio = SYSCTL_PERIPH_GPIOC, \
                            .pin_config_tx = GPIO_PC7_U3TX,  .pin_config_rx = GPIO_PC6_U3RX,                \
                            .gpio_pins = GPIO_PIN_6 | GPIO_PIN_7, .gpio_port_base = GPIO_PORTC_BASE,        \
                            .uart_base = UART3_BASE, .uart_int = INT_UART3                                  \
                        }
#define HW_UART4_INFO   {   .sys_periph_uart = SYSCTL_PERIPH_UART4, .sys_periph_gpio = SYSCTL_PERIPH_GPIOC, \
                            .pin_config_tx = GPIO_PC5_U4TX,  .pin_config_rx = GPIO_PC4_U4RX,                \
                            .gpio_pins = GPIO_PIN_4 | GPIO_PIN_5, .gpio_port_base = GPIO_PORTC_BASE,        \
                            .uart_base = UART4_BASE, .uart_int = INT_UART4                                  \
                        }
#define HW_UART5_INFO   {   .sys_periph_uart = SYSCTL_PERIPH_UART5, .sys_periph_gpio = SYSCTL_PERIPH_GPIOE, \
                            .pin_config_tx = GPIO_PE5_U5TX,  .pin_config_rx = GPIO_PE4_U5RX,                \
                            .gpio_pins = GPIO_PIN_4 | GPIO_PIN_5, .gpio_port_base = GPIO_PORTE_BASE,        \
                            .uart_base = UART5_BASE, .uart_int = INT_UART5                                  \
                        }

typedef struct {
    uint32_t    sys_periph_uart;
    uint32_t    sys_periph_gpio;
    uint32_t    pin_config_tx;
    uint32_t    pin_config_rx;
    uint8_t     gpio_pins;
    uint32_t    gpio_port_base;
    uint32_t    uart_base;
    uint32_t    uart_int;
} hw_uart_info_t;

typedef struct hw_uart_ {
    hw_uart_num_t       hw_uart_num;
    hw_uart_info_t      uart_info;
    uint16_t            max_buffer_size;
    uint8_t*            buffer;
    uint16_t            first;
    uint16_t            last;
    SemaphoreHandle_t   data_ready;
} hw_uart_t;

static hw_uart_handle_t p_uart_handle[5] = {NULL, NULL, NULL, NULL, NULL};

static hw_uart_info_t hw_uart_get_config_info(hw_uart_num_t uart_num)
{
    hw_uart_info_t uart_info;
    switch (uart_num) {
    case HW_UART_NUM_0:
        uart_info = (hw_uart_info_t)HW_UART0_INFO;
        break;
    case HW_UART_NUM_1:
        uart_info = (hw_uart_info_t)HW_UART1_INFO;
        break;
    case HW_UART_NUM_2:
        uart_info = (hw_uart_info_t)HW_UART2_INFO;
        break;
    case HW_UART_NUM_3:
        uart_info = (hw_uart_info_t)HW_UART3_INFO;
        break;
    case HW_UART_NUM_4:
        uart_info = (hw_uart_info_t)HW_UART4_INFO;
        break;
    case HW_UART_NUM_5:
        uart_info = (hw_uart_info_t)HW_UART5_INFO;
        break;
    }
    return uart_info;
}

void UARTIntHandler(void)
{
    uint32_t ui32Status;
    uint8_t i = 0;
    for (i = 0; i < 5; i++) {
        if (p_uart_handle[i] == NULL) {
            continue;
        }
        hw_uart_handle_t uart_handle = p_uart_handle[i];
        ui32Status = UARTIntStatus(uart_handle->uart_info.uart_base, true);
        if (ui32Status) {
            UARTIntClear(uart_handle->uart_info.uart_base, ui32Status);
            while(UARTCharsAvail(uart_handle->uart_info.uart_base)) //loop while there are chars
            {
                uart_handle->buffer[uart_handle->last] = UARTCharGetNonBlocking(uart_handle->uart_info.uart_base);
                uart_handle->last++;
                if (uart_handle->last >= uart_handle->max_buffer_size) {
                    uart_handle->last = 0;
                }
                if (uart_handle->last == uart_handle->first) {
                    uart_handle->first++;
                }
//                SysCtlDelay(SysCtlClockGet() / 3000);
            }
            BaseType_t* xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(uart_handle->data_ready, xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            break;
        }
    }
}

hw_uart_handle_t tiva_uart_init(hw_uart_config_t config)
{
    hw_uart_handle_t uart_handle = calloc(1, sizeof(hw_uart_t));
    if (uart_handle == NULL) {
        return NULL;
    }
    if (config.max_buffer_size == 0) {
        uart_handle->max_buffer_size = DEFAULT_MAX_BUFFER_SIZE;
    } else {
        uart_handle->max_buffer_size = config.max_buffer_size;
    }
    uart_handle->buffer = (uint8_t*)malloc(uart_handle->max_buffer_size);
    if (uart_handle->buffer == NULL) {
        free(uart_handle);
        return NULL;
    }
    uart_handle->uart_info = hw_uart_get_config_info(config.hw_uart_num);
    SysCtlPeripheralEnable(uart_handle->uart_info.sys_periph_uart);
    SysCtlPeripheralEnable(uart_handle->uart_info.sys_periph_gpio);

    GPIOPinConfigure(uart_handle->uart_info.pin_config_tx);
    GPIOPinConfigure(uart_handle->uart_info.pin_config_rx);
    GPIOPinTypeUART(uart_handle->uart_info.gpio_port_base, uart_handle->uart_info.gpio_pins);
    UARTConfigSetExpClk(uart_handle->uart_info.uart_base, SysCtlClockGet(), config.baudrate, config.frame_config);
    uart_handle->first = 0;
    uart_handle->last = 0;
    uart_handle->hw_uart_num = config.hw_uart_num;
    uart_handle->data_ready = xSemaphoreCreateBinary();
    p_uart_handle[config.hw_uart_num] = uart_handle;
    UARTIntRegister(uart_handle->uart_info.uart_base, &UARTIntHandler);
    IntMasterEnable(); //enable processor interrupts
    IntEnable(uart_handle->uart_info.uart_int); //enable the UART interrupt
    UARTIntEnable(uart_handle->uart_info.uart_base, UART_INT_RX | UART_INT_RT);
    return uart_handle;
}

void tiva_uart_write_bytes(hw_uart_handle_t uart_handle, uint8_t* buffer, uint16_t buffer_len)
{
    if (buffer == NULL || uart_handle == NULL) {
        return;
    }
    uint16_t i = 0;
    for (i = 0; i < buffer_len; i++) {
        UARTCharPut(uart_handle->uart_info.uart_base, buffer[i]);
    }
}

uint16_t tiva_uart_read_bytes(hw_uart_handle_t uart_handle, uint8_t* buffer, uint16_t buffer_len)
{
    if (!tiva_uart_data_available(uart_handle)) {
        return 0;
    }
    if (buffer == NULL || buffer_len == 0) {
        return 0;
    }
    uint16_t count = 0;
    while (uart_handle->first != uart_handle->last) {
        buffer[count] = uart_handle->buffer[uart_handle->first];
        uart_handle->first++;
        count++;
        if (uart_handle->first >= uart_handle->max_buffer_size) {
            uart_handle->first = 0;
        }
        if (count >= buffer_len) {
            break;
        }
    }
    if (uart_handle->first == uart_handle->last) {
        uart_handle->first = 0;
        uart_handle->last = 0;
    }
    return count;
}

bool tiva_uart_data_available(hw_uart_handle_t uart_handle)
{
    if (uart_handle->last == uart_handle->first) {
        return false;
    }
    return true;
}

char tiva_uart_read_char(hw_uart_handle_t uart_handle)
{
    char buffer;
    int len = tiva_uart_read_bytes(uart_handle, &buffer, 1);
    if (len) {
        return buffer;
    }
    return 0;
}

void tiva_uart_write_string(hw_uart_handle_t uart_handle, char* s_data)
{
    tiva_uart_write_bytes(uart_handle, (uint8_t*)s_data, strlen(s_data));
}

bool tiva_uart_wait_data(hw_uart_handle_t uart_handle, uint32_t tick)
{
    bool ret = false;
    if (xSemaphoreTake(uart_handle->data_ready, tick) == pdTRUE) {
        ret = true;
    }
    return ret;
}

