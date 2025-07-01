#include "gatt_server_handler.h"
#include "esp_log.h"

static const char *TAG = "GATT_SERVER";

void gatt_server_event_handler(esp_gatts_cb_event_t event,
                               esp_gatt_if_t gatts_if,
                               esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
        default:
            ESP_LOGI(TAG, "Unhandled GATT server event: %d", event);
            break;
    }
}
