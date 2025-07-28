#include "gap_handler.h"

static const char *TAG = "GAP_HANDLER";

// STATIC VARIABLES

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 0,
    .p_service_uuid = NULL,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static bool connection_status = false;

// GAP FUNCTIONS
void set_adv_data()
{
    esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set advertising data: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Advertising data set successfully");
    }
}

esp_err_t start_advertise()
{
    return esp_ble_gap_start_advertising(&adv_params);
}

void update_connection_status(bool connected)
{
    connection_status = connected;
}

// GAP EVENT HANDLER
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(TAG, "Advertising data set successfully");
            start_advertise();
            break;
        
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG, "Failed to start advertising: %s", esp_err_to_name(param->adv_start_cmpl.status));
                
                // Try restarting advertising after 1 second
                vTaskDelay(pdMS_TO_TICKS(100));

                esp_err_t retry = start_advertise();
                if (retry != ESP_OK) {
                    ESP_LOGE(TAG, "Retry advertising failed: %s", esp_err_to_name(retry));
                } else {
                    ESP_LOGI(TAG, "Retrying advertising...");
                }

            } else {
                ESP_LOGI(TAG, "Advertising started successfully");
            }
            break;

        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (connection_status)
            {
                ESP_LOGI(TAG, "Advertising stopped, device is connected");
            } else {
                ESP_LOGI(TAG, "Advertising stopped, no device connected");
                
                // Restart advertising if no device is connected
                esp_err_t ret = start_advertise();
                if (ret != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to restart advertising: %s", esp_err_to_name(ret));
                } else {
                    ESP_LOGI(TAG, "Restarting advertising after stop");
                }
            }
            break;
        
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(TAG, "Connection params updated: intv [%d, %d], latency %d, timeout %d",
                     param->update_conn_params.min_int,
                     param->update_conn_params.max_int,
                     param->update_conn_params.latency,
                     param->update_conn_params.timeout);
            break;
        
        case ESP_GAP_BLE_SEC_REQ_EVT:
            ESP_LOGI(TAG, "Security request received");
            esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
            break;
        
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            if (param->ble_security.auth_cmpl.success) {
                ESP_LOGI(TAG, "Authentication successful");
                ESP_LOG_BUFFER_HEX(TAG, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
            } else {
                ESP_LOGE(TAG, "Authentication failed, reason: %d", param->ble_security.auth_cmpl.fail_reason);
            }
            break;
        
        case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
            ESP_LOGI(TAG, "Unbonded device successfully");
            break;
        
        case ESP_GAP_BLE_KEY_EVT:
            ESP_LOGI(TAG, "Key event received, type: %d", param->ble_security.ble_key.key_type);
            break;
        
        case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
            ESP_LOGI(TAG, "Packet length set successfully");
            break;
        
        default:
            ESP_LOGI(TAG, "Unhandled GAP event: %d", event);
            break;
    }
}
