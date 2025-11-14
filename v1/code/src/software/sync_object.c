#include "sync_object.h"

#include "FreeRTOS.h"
#include "semphr.h"

void * sync_object_init(void )
{
    return xSemaphoreCreateMutex();
}

void sync_object_take(void * obj)
{
    xSemaphoreTake(obj, portMAX_DELAY);
}

void sync_object_release(void * obj)
{
     xSemaphoreGive(obj);
}
