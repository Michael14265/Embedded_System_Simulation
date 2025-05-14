/* Standard includes. */
#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include "assert.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "publics.h"

/* Local Defines */
#define HISTORY_DEPTH 8
#define WAIT_FOREVER 0

/* Local Structures */
typedef struct
{
    int a_iLevel[HISTORY_DEPTH];  /* Tank level */
    int aa_iTime[HISTORY_DEPTH][4]; /* Time level was measured */
    int iCurrent;  /* Index to most recent entry */
    BOOL fFull;  /* TRUE if all history entries have data */
} TANK_DATA;

/* Static Data */

/* Data about each of the tanks */
static TANK_DATA a_td[COUNTOF_TANKS];

SemaphoreHandle_t xSemData;

void vTankDataInit(void) {
    int iTank;

    /* Note that all the history tables are empty */
    for (iTank = 0; iTank < COUNTOF_TANKS; ++iTank)
    {
        a_td[iTank].iCurrent = -1;
        a_td[iTank].fFull = FALSE;
    }

    /* Initialize the semaphore that protects the data */
    xSemData = xSemaphoreCreateBinary();
    xSemaphoreGive(xSemData);
}

void vTankDataAdd(int iTank, int iLevel) {
    assert(iTank >= 0 && iTank < COUNTOF_TANKS);

    xSemaphoreTake(xSemData, portMAX_DELAY);

    /* Go to the next data entry in the tank */
    ++a_td[iTank].iCurrent;

    /* If data array is full, set appropriate flag */
    if (a_td[iTank].iCurrent == HISTORY_DEPTH) {
        a_td[iTank].iCurrent = 0;
        a_td[iTank].fFull = TRUE;
    }

    /* Put the data in place */
    a_td[iTank].a_iLevel[a_td[iTank].iCurrent] = iLevel;
    vTimeGet(a_td[iTank].aa_iTime[a_td[iTank].iCurrent]);

    xSemaphoreGive(xSemData);

    vDisplayUpdate();
}

int iTankDataGet(int iTank, int* a_iLevels, int** aa_iTimes, int iLimit) {
    int iReturn;
    int iIndex;

    assert(iTank >= 0 && iTank < COUNTOF_TANKS);
    assert(a_iLevels != NULL);
    assert(iLimit > 0);

    iReturn = 0;

    if (iLimit > HISTORY_DEPTH)
        iLimit = HISTORY_DEPTH;

    xSemaphoreTake(xSemData, portMAX_DELAY);

    iIndex = a_td[iTank].iCurrent;

    while (iIndex >= 0 && iReturn < iLimit)
    {
        /* Get the next entry into the array */
        a_iLevels[iReturn] = a_td[iTank].a_iLevel[iIndex];

        /* Get the time, if the caller asked for it */
        if (aa_iTimes != NULL)
        {
            aa_iTimes[iReturn * 4 + 0] = a_td[iTank].aa_iTime[iIndex][0];
            aa_iTimes[iReturn * 4 + 1] = a_td[iTank].aa_iTime[iIndex][1];
            aa_iTimes[iReturn * 4 + 2] = a_td[iTank].aa_iTime[iIndex][2];
            aa_iTimes[iReturn * 4 + 3] = a_td[iTank].aa_iTime[iIndex][3];
        }
        ++iReturn;

        /* Find the next oldest element in the array */
        --iIndex;
        /* If the current pointer has wrapped */
        if (iIndex == - 1 && a_td[iTank].fFull)
        {
            /* ... go back to the end of the array */
            iIndex = HISTORY_DEPTH - 1;
        }
    }

    xSemaphoreGive(xSemData);

    return(iReturn);
}