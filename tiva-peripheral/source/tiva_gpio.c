/*
 * tiva_gpio.c
 *
 *  Created on: Dec 2, 2018
 *      Author: toright
 */

#include "tiva_gpio.h"
#include "tiva_periph.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "string.h"

typedef struct tiva_gpio_ {
    uint32_t    sysctl_periph;
    uint32_t    gpio_base;
    uint8_t     gpio_pin;
} tiva_gpio_t;

tiva_gpio_handle_t tiva_gpio_init(tiva_gpio_config_t gpio_config)
{
    tiva_gpio_handle_t gpio_handle = calloc(1, sizeof(tiva_gpio_t));
    gpio_handle->sysctl_periph = gpio_config.sysctl_periph;
    gpio_handle->gpio_base = gpio_config.gpio_base;
    gpio_handle->gpio_pin = gpio_config.gpio_pin;
    SysCtlPeripheralEnable(gpio_handle->sysctl_periph);
    if (gpio_config.gpio_mode == GPIO_MODE_OUTPUT) {
        GPIOPinTypeGPIOOutput(gpio_handle->gpio_base, gpio_handle->gpio_pin);
        GPIOPinWrite(gpio_handle->gpio_base, gpio_handle->gpio_pin, 0);
    } else if (gpio_config.gpio_mode == GPIO_MODE_INPUT) {
        if (gpio_handle->gpio_base == GPIO_PORTF_BASE && gpio_handle->gpio_pin == GPIO_PIN_0) {
            HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
            HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
            HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
        }
        GPIODirModeSet(gpio_handle->gpio_base,  gpio_handle->gpio_pin, GPIO_DIR_MODE_IN);
        GPIOPadConfigSet(gpio_handle->gpio_base, gpio_handle->gpio_pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    }
    return gpio_handle;
}

void tiva_gpio_set_level(tiva_gpio_handle_t gpio_handle, bool level)
{
    if (level) {
        GPIOPinWrite(gpio_handle->gpio_base, gpio_handle->gpio_pin, gpio_handle->gpio_pin);
    } else {
        GPIOPinWrite(gpio_handle->gpio_base, gpio_handle->gpio_pin, 0);
    }
}

void tiva_gpio_toggle_level(tiva_gpio_handle_t gpio_handle)
{
    uint8_t cur = GPIOPinRead(gpio_handle->gpio_base, gpio_handle->gpio_pin);
    if (cur == 0) {
        GPIOPinWrite(gpio_handle->gpio_base, gpio_handle->gpio_pin, gpio_handle->gpio_pin);
    } else {
        GPIOPinWrite(gpio_handle->gpio_base, gpio_handle->gpio_pin, 0);
    }
}

bool tiva_gpio_get_level(tiva_gpio_handle_t gpio_handle) {
    if (GPIOPinRead(gpio_handle->gpio_base, gpio_handle->gpio_pin) == 0) {
        return LOW;
    } else {
        return HIGH;
    }
}




