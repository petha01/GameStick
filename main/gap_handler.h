#ifndef GAP_HANDLER_H
#define GAP_HANDLER_H

#include "esp_gap_ble_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

esp_err_t start_advertise();
void set_adv_data();
void update_connection_status(bool connected);

#endif // GAP_HANDLER_H
