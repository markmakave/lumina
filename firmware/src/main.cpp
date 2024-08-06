#include <iostream>

#include <esp_log.h>
#include <esp_camera.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


#define TAG "lumina"

extern "C"
int app_main()
{

    while (true)
    {
        ESP_LOGI(TAG, "Starting camera");

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    return 0;
}
