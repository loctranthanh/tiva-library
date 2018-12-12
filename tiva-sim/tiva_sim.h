/*
 * tiva-sim.h
 *
 *  Created on: Dec 9, 2018
 *      Author: toright
 */

#ifndef _TIVA_SIM_H_
#define _TIVA_SIM_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "tiva_uart.h"

#define TIVA_SIM_BUFFER_SIZE            512
#define TIVA_SIM_PHONE_NUMER_SIZE       15
#define TIVA_SIM_MESSAGE_SIZE           100
#define MANTAINER_PHONE                 "+84326577774"

typedef struct {
    hw_uart_config_t    uart_config;
} tiva_sim_config_t;

typedef struct {
    char sender_phone[TIVA_SIM_PHONE_NUMER_SIZE];
    char message[TIVA_SIM_MESSAGE_SIZE];
} sim_message_info_t;

typedef struct tiva_sim_* tiva_sim_handle_t;

tiva_sim_handle_t tiva_sim_init(tiva_sim_config_t sim_config);

void tiva_sim_run(tiva_sim_handle_t sim_handle);

void tiva_sim_destroy(tiva_sim_handle_t sim_handle);

bool tiva_sim_send_message(tiva_sim_handle_t sim_handle, char* message, const char* phone_num);

bool tiva_sim_read_message(tiva_sim_handle_t sim_handle, sim_message_info_t* message_info);

#endif /* _TIVA_SIM_H_ */
