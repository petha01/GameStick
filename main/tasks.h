#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void task_send_ble_packets();
void create_send_ble_packets();
void suspend_send_ble_packets();
void resume_send_ble_packets();
void delete_send_ble_packets();