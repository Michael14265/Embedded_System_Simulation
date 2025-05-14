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
#define MSG_UPDATE           0x0001
#define MSG_DISP_RESET_ALARM 0x0002
#define MSG_DISP_NO_PROMPT   0x0003
#define MSG_DISP_OFLOW       0x0040
#define MSG_DISP_LEAK        0x0080
#define MSG_USER_REQUEST     0x8000
#define MSG_DISP_TIME        (MSG_USER_REQUEST | 0x0001)
#define MSG_DISP_TANK        (MSG_USER_REQUEST | 0x0400)
#define MSG_DISP_PROMPT      (MSG_USER_REQUEST | 0x0800)

static void vDisplayTask(void* pvParameters);

/* The stack and input queue for the display task */
#define STK_SIZE 1024
//static UWORD DisplayTaskStk[STK_SIZE];

#define Q_SIZE 10
QueueHandle_t displayQueue;
/*static OS_EVENT* QDisplayTask;
static void* a_pvQDisplayData[Q_SIZE];*/

void vDisplaySystemInit(/*INPUTS:*/void) {

    displayQueue = xQueueCreate(Q_SIZE, sizeof(WORD));

    xTaskCreate(vDisplayTask, "displaytask", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY_DISPLAY, NULL);
}

static void vDisplayTask(void* pvParameters) {

    WORD wMsg;
    int a_iTime[4];
    char a_chDisp[21];
    WORD wUserRequest;
    int iLevel;
    int iTankLeaking;
    int iTankOverflow;
    int iPrompt;

    /* Initialize the display */
    vTimeGet(a_iTime);
    sprintf(a_chDisp, "%02d:%02d:%02d", a_iTime[0], a_iTime[1], a_iTime[2]);
    vHardwareDisplayLine(a_chDisp);
    wUserRequest = MSG_DISP_TIME;

    iTankLeaking = NO_TANK;
    iTankOverflow = NO_TANK;
    iPrompt = -1;

    while (TRUE) {
        xQueueReceive(displayQueue, &wMsg, portMAX_DELAY);

        if (wMsg & MSG_USER_REQUEST)
        {
            if (wMsg & MSG_USER_REQUEST & MSG_DISP_PROMPT)
            {
                /* Store the prompt we’ve been asked to display */
                iPrompt = wMsg & MSG_DISP_PROMPT;
            }
            else
            {
                /* Store what the user asked us to display */
                wUserRequest = wMsg;
            }
        }
        else if (wMsg & MSG_DISP_LEAK)
        {
            /* Store the number of the leaking tank */
            iTankLeaking = wMsg - MSG_DISP_LEAK;
        }
        else if (wMsg & MSG_DISP_OFLOW)
        {
            /* Store the number of the overflowing tank */
            iTankOverflow = wMsg - MSG_DISP_OFLOW;
        }
        else if (wMsg == MSG_DISP_RESET_ALARM)
        {
            iTankLeaking = NO_TANK;
            iTankOverflow = NO_TANK;
            iPrompt = -1;
        }
        else if (wMsg == MSG_DISP_NO_PROMPT)
        {
            iPrompt = -1;
        }

        /* ELSE it’s an update message */
        /* Now do the display */
        if (iTankOverflow != NO_TANK)
        {
            /* A tank is overflowing. This takes priority */
            sprintf(a_chDisp, "Tank %d: OVERFLOW!!", iTankOverflow + 1);
        }
        else if (iTankLeaking != NO_TANK)
        {
            /* A tank is leaking. */
            sprintf(a_chDisp, "Tank %d: LEAKING!!", iTankLeaking + 1);
        }
        else if (iPrompt > 0)
        {
            sprintf(a_chDisp, p_chGetCommandPrompt(iPrompt));
        }
        else if (wUserRequest == MSG_DISP_TIME)
        {
            /* Display the time */
            vTimeGet(a_iTime);
            sprintf(a_chDisp, "%02d:%02d:%02d",
                a_iTime[0], a_iTime[1], a_iTime[2]);
        }
        else
        {
            /* User must want tank level displayed. Get a level */
            if (iTankDataGet(wUserRequest - MSG_DISP_TANK, &iLevel, NULL, 1) == 1)
            {
                /* We have data for this tank; display it */
                sprintf(a_chDisp, "Tank %d: %d gls.", wUserRequest - MSG_DISP_TANK + 1, iLevel);
            }
            else
            {
                /* A level for this tank is not yet available */
                sprintf(a_chDisp, "Tank %d: N/A.", wUserRequest - MSG_DISP_TANK + 1);
            }
        }
        vHardwareDisplayLine(a_chDisp);
    }
}

void vDisplayUpdate(void) {
    WORD msg = MSG_UPDATE;
    xQueueSend(displayQueue,&msg,portMAX_DELAY);
}

void vDisplayTankLevel(int iTank) {
    assert(iTank >= 0 && iTank < COUNTOF_TANKS);
    WORD msg = MSG_DISP_TANK + iTank;
    xQueueSend(displayQueue, &msg, portMAX_DELAY);
}

void vDisplayTime(void) {
    WORD msg = MSG_DISP_TIME;
    xQueueSend(displayQueue, &msg, portMAX_DELAY);
}

void vDisplayPrompt(int iPrompt) {
    assert(iPrompt < 0x400);
    WORD msg = MSG_DISP_PROMPT + iPrompt;
    xQueueSend(displayQueue, &msg, portMAX_DELAY);
}

void vDisplayNoPrompt(void) {
    WORD msg = MSG_DISP_NO_PROMPT;
    xQueueSend(displayQueue, &msg, portMAX_DELAY);
}

void vDisplayLeak(int iTank) {
    assert(iTank >= 0 && iTank < COUNTOF_TANKS);
    WORD msg = MSG_DISP_LEAK + iTank;
    xQueueSend(displayQueue, &msg, portMAX_DELAY);
}

/* This routine is called when an overflow is detected */
void vDisplayOverflow(int iTank)
{
    /* Check that the parameter is valid */
    assert(iTank >= 0 && iTank < COUNTOF_TANKS);
    WORD msg = MSG_DISP_OFLOW + iTank;
    xQueueSend(displayQueue, &msg, portMAX_DELAY);
}

void vDisplayResetAlarm(void) {
    WORD msg = MSG_DISP_RESET_ALARM;
    xQueueSend(displayQueue, &msg, portMAX_DELAY);
}