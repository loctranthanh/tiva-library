/*
 * tiva_periph.c
 *
 *  Created on: Dec 2, 2018
 *      Author: toright
 */

#include "tiva_periph.h"
#include "driverlib/systick.h"

typedef struct tiva_periph_ {
    uint32_t systick_counter;
} tiva_periph_t;

typedef struct tiva_periph_* tiva_periph_handle_t;

tiva_periph_handle_t g_periph_handle = NULL;

static void tiva_periph_systick_isr()
{
    g_periph_handle->systick_counter++;
}

static void tiva_periph_systick_init(uint16_t num_tick)
{
    SysTickPeriodSet(SysCtlClockGet() / 1000 * num_tick);
    SysTickIntRegister(&tiva_periph_systick_isr);
    SysTickIntEnable();
    SysTickEnable();
}

void tiva_periph_init()
{
    g_periph_handle = calloc(1, sizeof(tiva_periph_t));
    tiva_periph_systick_init(1);
}

uint32_t tiva_periph_get_tick()
{
    if (g_periph_handle == NULL) {
        return 0;
    }
    return g_periph_handle->systick_counter;
}



