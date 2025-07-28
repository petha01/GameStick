#include "tasks.h"

static const char *TAG = "TASKS";

static TaskHandle_t task_handle_send_ble_packets = NULL;

void task_send_ble_packets()
{
    ESP_LOGI(TAG, "Intializing BLE packet send task...");

    while (1) {
        // Simulated work
        ESP_LOGI(TAG, "Running BLE packet send task");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void create_send_ble_packets()
{
    BaseType_t ret = xTaskCreate(task_send_ble_packets, "send_ble_packets", 2048, NULL, 5, &task_handle_send_ble_packets);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create BLE packet send task");
    } else {
        ESP_LOGI(TAG, "BLE packet send task created successfully");
        suspend_send_ble_packets();
    }
}

void suspend_send_ble_packets()
{
    if (task_handle_send_ble_packets != NULL) {
        vTaskSuspend(task_handle_send_ble_packets);
        ESP_LOGI(TAG, "BLE packet send task suspended");
    } else {
        ESP_LOGE(TAG, "BLE packet send task not created or already suspended");
    }
}

void resume_send_ble_packets()
{
    if (task_handle_send_ble_packets != NULL) {
        vTaskResume(task_handle_send_ble_packets);
        ESP_LOGI(TAG, "BLE packet send task resumed");
    } else {
        ESP_LOGE(TAG, "BLE packet send task not created or already running");
    }
}

void delete_send_ble_packets()
{
    if (task_handle_send_ble_packets != NULL) {
        vTaskDelete(task_handle_send_ble_packets);
        task_handle_send_ble_packets = NULL;
        ESP_LOGI(TAG, "BLE packet send task deleted");
    } else {
        ESP_LOGE(TAG, "BLE packet send task not created or already deleted");
    }
}