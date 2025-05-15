/*************************************************************

                         BUTTON.C

This module deals with the buttons.

*************************************************************/

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
#include "assert.h"

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
    enum CMD_STATE iCmdState;
    
    /* Prevent the compiler warning about the unused parameter. */
    (void)pvParameters;

    iCmdState = CMD_NONE;

    for (;;) {
        xQueueReceive(buttonQueue, &wMsg, portMAX_DELAY);

        switch (iCmdState) {
            case CMD_NONE:
                switch (wMsg)
                {
                    case '1':
                    case '2':
                    case '3':
                        vDisplayTankLevel(wMsg - '1');
                        break;

                    case 'T':
                        vDisplayTime();
                        break;
                    case 'R':
                        vHardwareBellOff();
                        vDisplayResetAlarm();
                        break;

                    case 'P':
                        iCmdState = CMD_PRINT;
                        vDisplayPrompt(0);
                        break;
                }
            case CMD_PRINT:
                switch (wMsg)
                {
                case 'R':
                    iCmdState = CMD_NONE;
                    vHardwareBellOff();
                    vDisplayResetAlarm();
                    break;

                case 'A':
                    vPrintAll();
                    iCmdState = CMD_NONE;
                    vDisplayNoPrompt();
                    break;

                case 'H':
                    iCmdState = CMD_PRINT_HIST;
                    vDisplayPrompt(1);
                    break;
                }
                break;

            case CMD_PRINT_HIST:
                switch (wMsg)
                {
                case 'R':
                    iCmdState = CMD_NONE;
                    vHardwareBellOff();
                    vDisplayResetAlarm();
                    break;

                case '1':
                case '2':
                case '3':
                    vPrintTankHistory(wMsg - '1');
                    iCmdState = CMD_NONE;
                    vDisplayNoPrompt();
                    break;
                }
                break;
        }
    }
}

void vButtonInterrupt(void) {
    WORD wButton;

    wButton = wHardwareButtonFetch();

    xQueueSendToBackFromISR(buttonQueue, &wButton, pdFALSE);

}

static char* p_chPromptStrings[] =
{
    "Press: HST or ALL",
    "Press Tank Number"
};

char* p_chGetCommandPrompt(int iPrompt) {
    assert(iPrompt >= 0 && iPrompt < sizeof(p_chPromptStrings) / sizeof(char*));

    return (p_chPromptStrings[iPrompt]);
}



