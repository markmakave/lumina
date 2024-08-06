
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "vec.hpp"

#define TAG "lumina"

void task(void*)
{
    return;
}

extern "C"
int app_main()
{
    xTaskCreate(
        task,
        "main",
        4096,
        NULL,
        5,
        NULL
    );

    return 0;
}
