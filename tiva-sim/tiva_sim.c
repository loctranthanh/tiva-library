/*
 * tiva_sim.c
 *
 *  Created on: Dec 9, 2018
 *      Author: toright
 */

#include "tiva_sim.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stdbool.h"
#include "string.h"
#include "tiva_log.h"

#define MAX_MESSAGE_IN_QUEUE    10
#define TAG                     "SIM"

typedef struct tiva_sim_ {
    hw_uart_handle_t    uart_handle;
    char*               buffer;
    bool                run;
    SemaphoreHandle_t   message_lock;
    bool                new_message_ready;
    sim_message_info_t  message_received;
    QueueHandle_t       message_queue;
} tiva_sim_t;

tiva_sim_handle_t tiva_sim_init(tiva_sim_config_t sim_config)
{
    tiva_sim_handle_t sim_handle = calloc(1, sizeof(tiva_sim_t));
    if (sim_handle == NULL) {
        return NULL;
    }
    sim_handle->uart_handle = tiva_uart_init(sim_config.uart_config);
    sim_handle->buffer = (char*)malloc(TIVA_SIM_BUFFER_SIZE);
    if (sim_handle->buffer == NULL) {
        free(sim_handle->uart_handle);
        free(sim_handle);
        return NULL;
    }
    sim_handle->run = false;
    sim_handle->message_queue = xQueueCreate(MAX_MESSAGE_IN_QUEUE, sizeof(sim_message_info_t));
    sim_handle->message_lock = xSemaphoreCreateMutex();
    sim_handle->new_message_ready = false;
    return sim_handle;
}

static bool _at_and_get_response(tiva_sim_handle_t sim_handle, const char *cmd, const char *expect, char *out, int timeout, int retry)
{

  do {
    tiva_uart_write_string(sim_handle->uart_handle, (char*)cmd);
    if (expect == NULL) {
        return true;
    }
    int recv_len = 0;
    while (tiva_uart_wait_data(sim_handle->uart_handle, timeout / portTICK_RATE_MS)) {
        recv_len += tiva_uart_read_bytes(sim_handle->uart_handle,
                                         (uint8_t*)(sim_handle->buffer + recv_len), TIVA_SIM_BUFFER_SIZE);
        if (recv_len >= TIVA_SIM_BUFFER_SIZE) {
            break;
        }
    }
    if (recv_len > 0) {
        sim_handle->buffer[recv_len] = '\0';
        TIVA_LOGI(TAG, "Rec: %s", sim_handle->buffer);
        memcpy(out, sim_handle->buffer, recv_len + 1);
        return true;
    }
    if (retry > 0) {
        retry --;
    }
  } while (retry);
  return false;
}

static bool _at_and_expect(tiva_sim_handle_t sim_handle, const char *cmd, const char *expect, int timeout, int retry)
{
  char out[100];
  do {
    if (_at_and_get_response(sim_handle, cmd, expect, out, timeout, 1)) {
        if (strstr(out, expect) != NULL) {
            return true;
        }
    }
    if (expect == NULL) {
        return true;
    }

    if (retry > 0) {
      retry --;
    }
  } while (retry);
  return false;
}

// +CMT: "+84388481545","","18/12/11,00:20:18+28"
static int get_sender_number(char* rec_buffer, char* sender_number)
{
    if (rec_buffer == NULL) {
        return 0;
    }
    char *start = strstr(rec_buffer, "+CMT:");
    if (start == NULL) {
        return 0;
    }
    start = start + 7;
    char* end = start + 1;
    while (*end >= '0' && *end <= '9') {
        end = end + 1;
    }
    end--;
    memcpy(sender_number, start, end - start + 1);
    return (end - start + 1);
}

// +CMT: "+84388481545","","18/12/11,00:20:18+28"
static int get_content_message(char* rec_buffer, char* content)
{
    if (rec_buffer == NULL) {
        return 0;
    }
    char *start = strstr(rec_buffer, "+CMT:");
    if (start == NULL) {
        return 0;
    }
    start = strstr(start, "\r\n");
    if (start == NULL) {
        return 0;
    }
    start = start + 2;
    char* end = start + 1;
    while (*end != '\r') {
        end = end + 1;
    }
    end--;
    memcpy(content, start, end - start + 1);
    return (end - start + 1);
}

bool is_new_message(char* message)
{
    if (strstr(message, "+CMT:") != NULL) {
        return true;
    }
    return false;
}

static bool tiva_sim_process_send_message(tiva_sim_handle_t sim_handle, char* message, const char* phone_num)
{
    if (!_at_and_expect(sim_handle, "AT\r\n", "OK", 200, 3)) {
        TIVA_LOGI(TAG, "fail AT");
        return false;
    }
    if (!_at_and_expect(sim_handle, "AT+CMGF=1\r\n", "OK", 200, 3)) {
        TIVA_LOGI(TAG, "fail AT+CMGF=1");
        return false;
    }
    char s_phone[30];
    int len = sprintf(s_phone, "AT+CMGS=\"%s\"\r\n", phone_num);
    s_phone[len] = '\0';
    if (!_at_and_expect(sim_handle, s_phone, ">", 200, 3)) {
        TIVA_LOGI(TAG,"fail CMGS");
        return false;
    }
    _at_and_expect(sim_handle, message, NULL, 200, 3);
    vTaskDelay(1000 / portTICK_RATE_MS);
    uint8_t c_end[3];
    c_end[0] = 26;
    tiva_uart_write_bytes(sim_handle->uart_handle, c_end, 1);
    _at_and_expect(sim_handle, "\r\n", "OK", 200, 3);
    return true;
}

static void tiva_sim_task(void *pv)
{
    tiva_sim_handle_t sim_handle = (tiva_sim_handle_t)pv;
     // setup module sim
    while (1) {
        if (!_at_and_expect(sim_handle, "AT+CREG?\r\n", "+CREG: 0,1", 1000, 5)) {
            TIVA_LOGI(TAG,"fail CREG");
            continue;
        }
        if (!_at_and_expect(sim_handle, "ATE0\r\n", "OK", 1000, 5)) {
            TIVA_LOGI(TAG,"fail ATE0");
            continue;
        }
        if (!_at_and_expect(sim_handle, "AT+CNMI=2,2,2,0,0\r\n", "OK", 1000, 5)) {
            TIVA_LOGI(TAG,"fail AT");
            continue;
        }
        break;
    }
    // send message to test
//    tiva_sim_process_send_message(sim_handle, "Device is ready!", MANTAINER_PHONE);
    int recv_len = 0;
    sim_message_info_t message_send;
    while (sim_handle->run)
    {
        if (xQueueReceive(sim_handle->message_queue, &message_send, 100 / portTICK_RATE_MS) == pdTRUE) {
            tiva_sim_process_send_message(sim_handle, message_send.message, message_send.sender_phone);
        }
        recv_len = 0;
        while (tiva_uart_wait_data(sim_handle->uart_handle, 200 / portTICK_RATE_MS)) {
            recv_len += tiva_uart_read_bytes(sim_handle->uart_handle,
                                             (uint8_t*)(sim_handle->buffer + recv_len), TIVA_SIM_BUFFER_SIZE);
            if (recv_len >= TIVA_SIM_BUFFER_SIZE) {
                break;
            }
        }
        if (recv_len > 0) {
            sim_handle->buffer[recv_len] = '\0';
            TIVA_LOGI(TAG, "Receive : %s", sim_handle->buffer);
            if (is_new_message(sim_handle->buffer)) {
                xSemaphoreTake(sim_handle->message_lock, portMAX_DELAY);
                int sender_num_len = get_sender_number(sim_handle->buffer, sim_handle->message_received.sender_phone);
                if (sender_num_len == 0 || sender_num_len > 15 || sender_num_len < 10) {
                    continue;
                }
                sim_handle->message_received.sender_phone[sender_num_len] = '\0';
                TIVA_LOGI(TAG, "sender number: %s", sim_handle->message_received.sender_phone);
                int content_len = get_content_message(sim_handle->buffer, sim_handle->message_received.message);
                if (content_len == 0 || content_len > TIVA_SIM_MESSAGE_SIZE) {
                    continue;
                }
                sim_handle->message_received.message[content_len] = '\0';
                TIVA_LOGI(TAG, "message content: %s", sim_handle->message_received.message);
                sim_handle->new_message_ready = true;
                xSemaphoreGive(sim_handle->message_lock);
            }
        }
    }

    vTaskDelete(NULL);
}

void tiva_sim_run(tiva_sim_handle_t sim_handle)
{
    sim_handle->run = true;
    xTaskCreate(tiva_sim_task, "tiva_sim_task", 256, (void*)sim_handle, 3, NULL);
}

bool tiva_sim_send_message(tiva_sim_handle_t sim_handle, char* message, const char* phone_num)
{
    sim_message_info_t new_message;
    strncpy(new_message.sender_phone, phone_num, sizeof(new_message.sender_phone));
    strncpy(new_message.message, message, sizeof(new_message.message));
    if (xQueueSend(sim_handle->message_queue, &new_message, 1000 / portTICK_RATE_MS) != pdTRUE) {
        return false;
    }
    return true;
}

bool tiva_sim_read_message(tiva_sim_handle_t sim_handle, sim_message_info_t* message_info)
{
    bool ret = false;
    if (sim_handle == NULL || message_info == NULL) {
        return false;
    }
    xSemaphoreTake(sim_handle->message_lock, portMAX_DELAY);
    if (sim_handle->new_message_ready) {
        memcpy(message_info, &sim_handle->message_received, sizeof(sim_message_info_t));
        ret = true;
        sim_handle->new_message_ready = false;
    }
    xSemaphoreGive(sim_handle->message_lock);
    return ret;
}

void tiva_sim_destroy(tiva_sim_handle_t sim_handle); // TODO:




