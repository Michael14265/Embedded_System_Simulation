/* Standard includes. */
#include <stdio.h>
#include <conio.h>
#include <Windows.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "publics.h"

/* Local Structures */
/* The state of the command state machine. */
enum CMD_STATE
{
    CMD_NONE,
    CMD_PRINT,
    CMD_PRINT_HIST
};

#define Q_SIZE 10

QueueHandle_t buttonQueue;

static void vButtonTask(void* pvParameters);

void vButtonSystemInit(void) {
    buttonQueue = xQueueCreate(Q_SIZE, sizeof(WORD));

    xTaskCreate(vButtonTask, "btn", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY_BUTTON, NULL);

}

static void vButtonTask(void* pvParameters) {
    
    WORD wMsg;
    
    /* Prevent the compiler warning about the unused parameter. */
    (void)pvParameters;

    for (;;) {
        xQueueReceive(buttonQueue, &wMsg, portMAX_DELAY);
        switch (wMsg)
        {

        default:
            taskENTER_CRITICAL();
            {
                printf("\r\nCharacter Entered: %c\r\n", wMsg);
            }
            taskEXIT_CRITICAL();
            break;
        }
    }
}

void vButtonInterrupt(void) {
    WORD wButton;

    wButton = wHardwareButtonFetch();

    xQueueSendToBackFromISR(buttonQueue, &wButton, pdFALSE);

}



