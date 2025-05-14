/****************************************************
                          PRINT.C
This module deals with the Tank Monitor printer.
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
#define MSG_PRINT_ALL        0x0020
#define MSG_PRINT_TANK_HIST  0x0010

/* Static Functions */
static void vPrinterTask(void* pvParameters);

/* Static Data */
/* The stack and input queue for the Printer task */
#define Q_SIZE 10
QueueHandle_t QPrinterTask;

/* Semaphore to wait for report to finish */
SemaphoreHandle_t semPrinter;

/* Place to construct report */
static char a_chPrint[10][21];

/* Count of lines in report */
static int iLinesTotal;

/* Count of lines printed so far */
static int iLinesPrinted;

/****** vPrinterSystemInit **********************************
This routine initializes the Printer system.

RETURNS: None.
***********************************************************/
void vPrinterSystemInit(void)
{
    QPrinterTask = xQueueCreate(Q_SIZE, sizeof(WORD));

    xTaskCreate(vPrinterTask, "prnt", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY_PRINTER, NULL);

    /* Initialize the semaphore as already taken */
    semPrinter = xSemaphoreCreateBinary();
}

/****** vPrinterTask ***************************************
This routine is the task that handles the Printer.

RETURNS: None.
***********************************************************/
static void vPrinterTask(void* pvParameters)
{
    /* LOCAL VARIABLES */
    #define MAX_HISTORY 5
    BYTE byErr;          /* Error code back from the OS */
    WORD wMsg;          /* Message received from the queue */
    int aa_iTime[MAX_HISTORY][4]; /* Time of day */
    int iTank;          /* Tank iterator */
    int a_iLevel[MAX_HISTORY]; /* Place to get level of tank */
    int iLevels;       /* Number of history level entries */
    int i;             /* The usual iterator */

    /* Keep the compiler warnings away */
    (void)pvParameters;

    while (TRUE)
    {
        /* Wait for a message */
        xQueueReceive(QPrinterTask, &wMsg, portMAX_DELAY);

        if (wMsg == MSG_PRINT_ALL)
        {
            /* Format 'all' report */
            iLinesTotal = 0;
            vTimeGet(aa_iTime[0]);
            sprintf(a_chPrint[iLinesTotal++],
                "Time: %02d:%02d:%02d",
                aa_iTime[0][0], aa_iTime[0][1], aa_iTime[0][2]);

            for (iTank = 0; iTank < COUNTOF_TANKS; ++iTank)
            {
                if (iTankDataGet(iTank, a_iLevel, NULL, 1) == 1)
                {
                    /* We have data for this tank; display it */
                    sprintf(a_chPrint[iLinesTotal++],
                        "Tank %d: %d gls.", iTank + 1, a_iLevel[0]);
                }
            }
            sprintf(a_chPrint[iLinesTotal++], "----------------");
            sprintf(a_chPrint[iLinesTotal++], " ");
        }
        else
        {
            /* Print the history of a single tank */
            iLinesTotal = 0;
            iTank = wMsg - MSG_PRINT_TANK_HIST;
            iLevels = iTankDataGet(iTank, a_iLevel, (int*)aa_iTime, MAX_HISTORY);
            sprintf(a_chPrint[iLinesTotal++], "Tank %d", iTank + 1);
            for (i = iLevels - 1; i >= 0; --i)
            {
                sprintf(a_chPrint[iLinesTotal++],
                    "%02d:%02d:%02d %4d gls.",
                    aa_iTime[i][0], aa_iTime[i][1], aa_iTime[i][2], a_iLevel[i]);
            }
            sprintf(a_chPrint[iLinesTotal++], "----------------");
            sprintf(a_chPrint[iLinesTotal++], " ");
        }

        iLinesPrinted = 0;
        vHardwarePrinterOutputLine(a_chPrint[iLinesPrinted++]);

        /* Wait for print job to finish */
        //OSSemPend(semPrinter, WAIT_FOREVER, &byErr);
        xSemaphoreTake(semPrinter,portMAX_DELAY);
    }
}

/****** vPrinterInterrupt **********************************
This routine is called when the printer interrupts.

RETURNS: None.
***********************************************************/
void vPrinterInterrupt(void)
{
    if (iLinesPrinted == iLinesTotal)
    {
        /* The report is done. Release the semaphore */
        xSemaphoreGive(semPrinter);
    }
    else
    {
        /* Print the next line */
        vHardwarePrinterOutputLine(a_chPrint[iLinesPrinted++]);
    }
}

/****** vPrintAll *****************************************
This routine is called when a printout is needed.
***********************************************************/
void vPrintAll(void)
{

    WORD msg = MSG_PRINT_ALL;
    xQueueSend(QPrinterTask, &msg, portMAX_DELAY);
}

/****** vPrintTankHistory **********************************
This routine is called when the user requests printing a tank level.
***********************************************************/
void vPrintTankHistory(int iTank)
{
    /* Check that the parameter is valid */
    assert(iTank >= 0 && iTank < COUNTOF_TANKS);

    WORD msg = MSG_PRINT_TANK_HIST + iTank;
    xQueueSend(QPrinterTask, &msg, portMAX_DELAY);
}