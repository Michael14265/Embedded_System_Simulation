/****************************************************
                          TIMER.C
This module provides timing services.
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

/* Static Data */
/* Data about the time */
static int iHours;
static int iMinutes;
static int iSeconds;
static int iSecondTenths;

/* The semaphore that protects the data */
SemaphoreHandle_t SemTime;

void vTimerInit(void) {
    /* Initialize the time */
    iHours = 0;
    iMinutes = 0;
    iSeconds = 0;
    iSecondTenths = 0;

    /* Initialize the semaphore */
    SemTime = xSemaphoreCreateBinary();
    xSemaphoreGive(SemTime);
}

void vTimerOneThirdSecond(void) {
    xSemaphoreTake(SemTime, portMAX_DELAY);

    //vOverflowTime();

    switch (iSecondTenths) {
        case 0:
            iSecondTenths = 3;
            break;

        case 3:
            iSecondTenths = 7;
            break;

        case 7:
            iSecondTenths = 0;
            ++iSeconds;
            if (iSeconds == 60) {
                iSeconds = 0;
                ++iMinutes;
            }
            if (iMinutes == 60) {
                iMinutes = 0;
                ++iHours;
            }
            if (iHours == 24) {
                iHours = 0;
            }

            vDisplayUpdate();
            break;
    }

    xSemaphoreGive(SemTime);
}

void vTimeGet(int* a_time) {
    
    xSemaphoreTake(SemTime, portMAX_DELAY);

    a_time[0] = iHours;
    a_time[1] = iMinutes;
    a_time[2] = iSeconds;
    a_time[3] = iSecondTenths;

    xSemaphoreGive(SemTime);
}