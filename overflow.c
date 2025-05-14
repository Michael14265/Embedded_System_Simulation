/****************************************************
                         OVERFLOW.C
This module deals with detecting overflows.
****************************************************/

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

/* Local Defines */
#define MSG_OFLOW_TIME       0xC010
#define MSG_OFLOW_ADD_TANK   0xC000

/* How long to watch tanks */
#define OFLOW_WATCH_TIME     (3 * 10)
#define OFLOW_THRESHOLD      7500

/* Local Structures */
typedef struct
{
    int iTime;   /* Time (in 1/3 seconds) to watch this tank */
    int iLevel;  /* Level last time this tank was checked */
} TANK_WATCH;

/* Static Functions */
static void vOverflowTask(void* pvParameters);
static void vFloatCallback(int iFloatLevel);

/* Static Data */
/* The stack and input queue for the Overflow task */
//#define STK_SIZE 1024
//static UWORD OverflowTaskStk[STK_SIZE];

#define Q_SIZE 10
QueueHandle_t QOverflowTask;


/****** vOverflowSystemInit *********************************
This routine initializes the Overflow system.

RETURNS: None.
***********************************************************/
void vOverflowSystemInit(void)
{

    /* Initialize the queue for this task. */
    QOverflowTask = xQueueCreate(Q_SIZE, sizeof(WORD));

    /* Start the task. */
    xTaskCreate(vOverflowTask, "ovrflw", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY_OVERFLOW, NULL);
}

/****** vOverflowTask ***************************************
This routine is the task that handles the Overflow.

RETURNS: None.
***********************************************************/
static far void vOverflowTask(void* pvParameters)
{
    /* LOCAL VARIABLES */
    BYTE byErr;           /* Error code back from the OS */
    WORD wMsg;           /* Message received from the queue */
    TANK_WATCH tw[3];    /* Structure with which to watch tanks */
    int i;              /* The usual iterator */
    int iTank;          /* Tank number to watch */
    int iFloatTank;     /* The tank whose float we're reading */

    /* Keep the compiler warnings away. */
    (void)pvParameters;

    /* We are watching no tanks */
    for (i = 0; i < 3; ++i)
        tw[i].iTime = 0;
    iFloatTank = 0;

    while (TRUE)
    {
        /* Wait for a message. */
        xQueueReceive(QOverflowTask, &wMsg, portMAX_DELAY);

        if (wMsg == MSG_OFLOW_TIME)
        {
            if (iFloatTank == 0)
            {
                /* Find the first tank on the watch list. */
                i = 0;
                while (i < COUNTOF_TANKS && !iFloatTank)
                {
                    if (tw[i].iTime != 0)
                    {
                        /* This tank is on the watch list */
                        /* Reduce the time for this tank. */
                        --tw[i].iTime;

                        /* Get the floats looking for the level
                           in this tank. */
                        iFloatTank = i;
                        vReadFloats(iFloatTank + 1, vFloatCallback);
                    }
                    ++i;
                }
            }
        }
        else if (wMsg >= MSG_OFLOW_ADD_TANK)
        {
            /* Add a tank to the watch list */
            iTank = wMsg - MSG_OFLOW_ADD_TANK;
            tw[iTank].iTime = OFLOW_WATCH_TIME;
            iTankDataGet(iTank, &tw[iTank].iLevel, NULL, 1);
        }
        else /* wMsg must be a float level. */
        {
            /* If the tank is still rising... */
            if (wMsg > tw[iFloatTank].iLevel)
            {
                /* If the level is too high... */
                if (wMsg >= OFLOW_THRESHOLD)
                {
                    /* Warn the user */
                    vHardwareBellOn();
                    vDisplayOverflow(iFloatTank);

                    /* Stop watching this tank */
                    tw[iFloatTank].iTime = 0;
                }
                else
                {
                    /* Keep watching it. */
                    tw[iFloatTank].iTime = OFLOW_WATCH_TIME;
                }
            }

            /* Store the new level */
            tw[iFloatTank].iLevel = wMsg;

            /* Find the first tank on the watch list. */
            i = iFloatTank;
            iFloatTank = 0;
            while (i < COUNTOF_TANKS && !iFloatTank)
            {
                if (tw[i].iTime != 0)
                {
                    /* This tank is on the watch list */
                    /* Reduce the time for this tank. */
                    --tw[i].iTime;

                    /* Get the floats looking for the level
                       in this tank. */
                    iFloatTank = i;
                    vReadFloats(iFloatTank, vFloatCallback);
                }
                ++i;
            }
        }
    }
}

/****** vFloatCallback *************************************
This is the routine that the floats module calls when it has
a float reading.

RETURNS: None.
***********************************************************/
static void vFloatCallback(int iFloatLevelNew)
{
    
    /* Put the level on the queue for the task. */
    WORD msg = iFloatLevelNew;
    xQueueSend(QOverflowTask, &msg, portMAX_DELAY);
}

/****** vOverflowTime **************************************
This routine is called three times a second.
***********************************************************/
void vOverflowTime(void)
{
    WORD msg = MSG_OFLOW_TIME;
    xQueueSend(QOverflowTask, &msg, portMAX_DELAY);
}

/****** vOverflowAddTank ***********************************
This routine is called when a tank level is increasing.
***********************************************************/
void vOverflowAddTank(int iTank)
{
    /* Check that the parameter is valid. */
    assert(iTank >= 0 && iTank < COUNTOF_TANKS);

    WORD msg = MSG_OFLOW_ADD_TANK + iTank;
    xQueueSend(QOverflowTask, &msg, portMAX_DELAY);
}