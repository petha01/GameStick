#include "gatt_server_handler.h"
#include "esp_log.h"

static const char *TAG = "GATT_SERVER";

void gatt_server_event_handler(esp_gatts_cb_event_t event,
                               esp_gatt_if_t gatts_if,
                               esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
        case ESP_GATTS_REG_EVT:
            ESP_LOGI(TAG, "GATT application registered, starting advertising");
            set_adv_data();
            break;
        case ESP_GATTS_CONNECT_EVT:
            update_connection_status(true);
            esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
            ESP_LOGI(TAG, "Device connected: %02x:%02x:%02x:%02x:%02x:%02x",
                     param->connect.remote_bda[0],
                     param->connect.remote_bda[1],
                     param->connect.remote_bda[2],
                     param->connect.remote_bda[3],
                     param->connect.remote_bda[4],
                     param->connect.remote_bda[5]);
            
            // Resume BLE packet sending task upon connection
            resume_send_ble_packets();
            break;

        case ESP_GATTS_DISCONNECT_EVT:
            update_connection_status(false);
            ESP_LOGI(TAG, "Device disconnected: %02x:%02x:%02x:%02x:%02x:%02x",
                     param->disconnect.remote_bda[0],
                     param->disconnect.remote_bda[1],
                     param->disconnect.remote_bda[2],
                     param->disconnect.remote_bda[3],
                     param->disconnect.remote_bda[4],
                     param->disconnect.remote_bda[5]);

            // Suspend BLE packet sending task upon disconnection
            suspend_send_ble_packets();
            
            // Restart advertising after disconnection
            esp_err_t ret = start_advertise();
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to restart advertising: %s", esp_err_to_name(ret));
            } else {
                ESP_LOGI(TAG, "Restarting advertising after disconnection");
            }
            break;
        
        case ESP_GATTS_MTU_EVT:
            ESP_LOGI(TAG, "MTU size set to %d", param->mtu.mtu);
            break;

        default:
            ESP_LOGI(TAG, "Unhandled GATT server event: %d", event);
            break;
    }
}
