#include <stdio.h>
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_check.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "gap_handler.h"
#include "gatt_server_handler.h"

static const char* TAG = "GameStick";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing ...");

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

    // Set device name
    ret = esp_ble_gap_set_device_name("ESP32_BLE");
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

    // Create GATT server
    // esp_ble_gatts_app_register(APP_ID);

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
    ret = nvs_flash_deinit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize NVS: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "NVS deinitialized successfully");
    }

    ESP_LOGI(TAG, "Exiting ...");
}
