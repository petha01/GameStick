#ifndef GATT_SERVER_HANDLER_H
#define GATT_SERVER_HANDLER_H

#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#include "gap_handler.h"
#include "tasks.h"

void gatt_server_event_handler(esp_gatts_cb_event_t event,
                               esp_gatt_if_t gatts_if,
                               esp_ble_gatts_cb_param_t *param);

#endif // GATT_SERVER_HANDLER_H