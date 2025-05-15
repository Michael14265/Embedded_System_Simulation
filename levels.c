/****************************************************
                          LEVELS.C
This module deals with calculating the tank levels.
****************************************************/

/* Standard includes. */
#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "publics.h"
#include "assert.h"

/* Local Defines */
#define MSG_LEVEL_VALUE 1

/* Static Functions */
/* The function to call when the floats have finished. */
static void vFloatCallback(int iFloatLevel);
static void vTestFloatCallback(int iFloatLevel);

/* The task. */
static void vLevelsTask(void* pvParameters);

/* Static Data */
/* Data for the message queue for the button task. */
#define Q_SIZE 10
QueueHandle_t QLevelsTask;

//#define STK_SIZE 1024
//static UWORD LevelsTaskStk[STK_SIZE];

/****** vLevelsSystemInit ***********************************
This routine is the task that initializes the levels task.

RETURNS: None.
***********************************************************/
void vLevelsSystemInit(void)
{
    /* Initialize the queue for this task. */
    QLevelsTask = xQueueCreate(Q_SIZE, sizeof(WORD));

    /* Start the task. */
    xTaskCreate(vLevelsTask, "lvls", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY_LEVELS, NULL);
}

/****** vLevelsTask *****************************************
This routine is the task that calculates the tank levels.

RETURNS: None.
***********************************************************/
static void vLevelsTask(void* pvParameters)
{
    /* LOCAL VARIABLES */
    BYTE byErr;           /* Error code back from the OS */
    WORD wFloatLevel;     /* Message received from the queue */
    int iTank;            /* Tank we're working on */
    int a_iLevels[3];     /* Levels for detecting leaks */

    /* Prevent the compiler warning about the unused parameter. */
    (void)pvParameters;

    /* Start with the first tank. */
    iTank = 0;

    while (TRUE)
    {
        /* Get the floats looking for the level in this tank. */
        vReadFloats(iTank, vFloatCallback);

        /* Wait for the result. */
        xQueueReceive(QLevelsTask, &wFloatLevel, portMAX_DELAY);

        /* The "calculation" wastes about 2 seconds. */
        clock_t start = clock();
        while (((double)(clock() - start) / CLOCKS_PER_SEC) < 2.0) {
            volatile int k = 0;
            for (int i = 0; i < 1000; i += 2)
                for (int j = 0; j < 1000; j += 2)
                    if ((i + j) % 2 != 0)
                        ++k;
        }

        /* Now that the "calculation" is done, assume that
           the number of gallons equals the float level. */
        vTankDataAdd(iTank, wFloatLevel);

        /* Now test for leaks (very simplistically). */
        if (iTankDataGet(iTank, a_iLevels, NULL, 3) == 3)
        {
            /* We got three levels. Test if the levels go down consistently. */
            if (a_iLevels[0] < a_iLevels[1] && a_iLevels[1] < a_iLevels[2])
            {
                vHardwareBellOn();
                vDisplayLeak(iTank);
            }

            /* If the tank is rising, watch for overflows. */
            if (a_iLevels[0] > a_iLevels[1])
                vOverflowAddTank(iTank);
        }

        /* Go to the next tank. */
        ++iTank;
        if (iTank == COUNTOF_TANKS)
            iTank = 0;
    }
}

/****** vFloatCallback **************************************
This is the routine that the floats module calls when it has
a float reading.

RETURNS: None.
***********************************************************/
static void vFloatCallback(int iFloatLevel)
{
    /* Put that button on the queue for the task. */
    WORD msg = iFloatLevel + MSG_LEVEL_VALUE;
    xQueueSendToBack(QLevelsTask, &msg, portMAX_DELAY);
}

static void vTestFloatCallback(int iFloatLevel) {
    printf("Test Float Call Back, Float Level = %d\n", iFloatLevel);
}