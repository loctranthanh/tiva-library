/*
 * tiva_gpio.h
 *
 *  Created on: Dec 2, 2018
 *      Author: toright
 */

#ifndef _TIVA_GPIO_H_
#define _TIVA_GPIO_H_


#include <stdint.h>
#include <stdbool.h>

#define HIGH        true
#define LOW         false

typedef enum {
    GPIO_MODE_INPUT = 0,
    GPIO_MODE_OUTPUT,
} gpio_mode_t;

typedef struct {
    uint32_t    sysctl_periph;
    uint32_t    gpio_base;
    uint8_t     gpio_pin;
    gpio_mode_t gpio_mode;
} tiva_gpio_config_t;

typedef struct tiva_gpio_* tiva_gpio_handle_t;

tiva_gpio_handle_t tiva_gpio_init(tiva_gpio_config_t gpio_config);

void tiva_gpio_set_level(tiva_gpio_handle_t gpio_handle, bool level);

void tiva_gpio_toggle_level(tiva_gpio_handle_t gpio_handle);

bool tiva_gpio_get_level(tiva_gpio_handle_t gpio_handle);

#endif /* _TIVA_GPIO_H_ */
