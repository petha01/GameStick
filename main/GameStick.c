#include "GameStick.h"

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing ...");

    SemaphoreHandle_t exit_semaphore;
    exit_semaphore = xSemaphoreCreateBinary();
    if (exit_semaphore == NULL) {
        ESP_LOGE(TAG, "Failed to create exit semaphore");
        return;
    }

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return;
    } else {
        ESP_LOGI(TAG, "NVS initialized successfully");
    }

    // Create and suspend tasks
    create_send_ble_packets();

    // Initialize BT Controller with default config
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth controller init failed: %s", esp_err_to_name(ret));
        goto uninit_nvs;
    } else {
        ESP_LOGI(TAG, "Bluetooth controller initialized successfully");
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth controller enable failed: %s", esp_err_to_name(ret));
        goto uninit_controller;
    } else {
        ESP_LOGI(TAG, "Bluetooth controller enabled successfully");
    }

    // Enable Bluetooth
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Bluetooth: %s", esp_err_to_name(ret));
        goto uninit_controller;
    } else {
        ESP_LOGI(TAG, "Bluetooth initialized successfully");
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable Bluetooth: %s", esp_err_to_name(ret));
        goto uninit_droid;
    } else {
        ESP_LOGI(TAG, "Bluetooth enabled successfully");
    }

    // Set security parameters
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
    uint8_t key_size = 16;
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t resp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;

    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &resp_key, sizeof(uint8_t));

    // Set device name
    ret = esp_ble_gap_set_device_name("ESP32 GameStick");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set device name: %s", esp_err_to_name(ret));
        goto uninit_droid;
    } else {
        ESP_LOGI(TAG, "Device name set successfully");
    }

    // Register Callbacks
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register GAP callback: %s", esp_err_to_name(ret));
        goto uninit_droid;
    } else {
        ESP_LOGI(TAG, "GAP callback registered successfully");
    }

    ret = esp_ble_gatts_register_callback(gatt_server_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register GATT server callback: %s", esp_err_to_name(ret));
        goto uninit_droid;
    } else {
        ESP_LOGI(TAG, "GATT server callback registered successfully");
    }

    // Register GATT application
    ret = esp_ble_gatts_app_register(APP_ID);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register GATT application: %s", esp_err_to_name(ret));
        goto uninit_droid;
    } else {
        ESP_LOGI(TAG, "GATT application registered successfully");
    }

    // Increase max bytes transferred?
    // esp_ble_gatt_set_local_mtu(200);

    // Exit starts here, use semaphore to wait
    xSemaphoreTake(exit_semaphore, portMAX_DELAY);

    // Disable Bluetooth
    ret = esp_bluedroid_disable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disable Bluetooth: %s", esp_err_to_name(ret));
    }

    // Uninitialize
uninit_droid:
    ret = esp_bluedroid_deinit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize Bluetooth: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Bluetooth deinitialized successfully");
    }

    ret = esp_bt_controller_disable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disable Bluetooth controller: %s", esp_err_to_name(ret));
    }

uninit_controller:
    ret = esp_bt_controller_deinit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize Bluetooth controller: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Bluetooth controller deinitialized successfully");
    }

uninit_nvs:

    // Delete tasks
    delete_send_ble_packets();
    
    // Deinitialize NVS
    ret = nvs_flash_deinit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize NVS: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "NVS deinitialized successfully");
    }

    ESP_LOGI(TAG, "Exiting ...");
}
