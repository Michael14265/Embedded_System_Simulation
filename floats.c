/****************************************************
                          FLOATS.C
This module deals with the float hardware.
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
#define WAIT_FOREVER  0

/* Static Data */
static V_FLOAT_CALLBACK vFloatCallback = NULL;
SemaphoreHandle_t xSemFloat;

/****** vFloatInit *****************************************
This routine is the task that initializes the float routines.

RETURNS: None.
***********************************************************/
void vFloatInit(void)
{
    /* Initialize the semaphore that protects the data. */
    xSemFloat = xSemaphoreCreateBinary();
    xSemaphoreGive(xSemFloat);
}

/****** vFloatInterrupt *************************************
This routine is the one that is called when the floats interrupt
with a new tank level reading.

RETURNS: None.
***********************************************************/
void vFloatInterrupt(void)
{
    /* LOCAL VARIABLES */
    int iFloatLevel;
    V_FLOAT_CALLBACK vFloatCallbackTemp;

    /* Get the float level. */
    iFloatLevel = iHardwareFloatGetData();

    /* Remember the callback function to call later. */
    vFloatCallbackTemp = vFloatCallback;
    vFloatCallback = NULL;

    /* We are no longer using the floats. Release the semaphore. */
    xSemaphoreGive(xSemFloat);

    /* Call back the callback routine. */
    vFloatCallbackTemp(iFloatLevel);
}

/****** vReadFloats *****************************************
This routine is the task that initializes the float routines.

RETURNS: None.
***********************************************************/
void vReadFloats(
    int iTankNumber,        /* The number of the tank to read. */
    V_FLOAT_CALLBACK vCb)   /* The function to call with the result. */
{

    /* Check that the parameter is valid. */
    assert(iTankNumber >= 0 && iTankNumber < COUNTOF_TANKS);

    xSemaphoreTake(xSemFloat, portMAX_DELAY);

    /* Set up the callback function */
    vFloatCallback = vCb;

    /* Get the hardware started reading the value. */
    vHardwareFloatSetup(iTankNumber);
}