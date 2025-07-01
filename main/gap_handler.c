#include "gap_handler.h"
#include "esp_log.h"

static const char *TAG = "GAP_HANDLER";

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        default:
            ESP_LOGI(TAG, "Unhandled GAP event: %d", event);
            break;
    }
}
