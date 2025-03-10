/*
 * FreeRTOS V202212.01
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

 /******************************************************************************
  * NOTE: Windows will not be running the FreeRTOS demo threads continuously, so
  * do not expect to get real time behaviour from the FreeRTOS Windows port, or
  * this demo application.  Also, the timing information in the FreeRTOS+Trace
  * logs have no meaningful units.  See the documentation page for the Windows
  * port for further information:
  * https://www.FreeRTOS.org/FreeRTOS-Windows-Simulator-Emulator-for-Visual-Studio-and-Eclipse-MingW.html
  *
  * NOTE 2:  This project provides two demo applications.  A simple blinky style
  * project, and a more comprehensive test and demo application.  The
  * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting in main.c is used to select
  * between the two.  See the notes on using mainCREATE_SIMPLE_BLINKY_DEMO_ONLY
  * in main.c.  This file implements the simply blinky version.  Console output
  * is used in place of the normal LED toggling.
  *
  * NOTE 3:  This file only contains the source code that is specific to the
  * basic demo.  Generic functions, such FreeRTOS hook functions, are defined
  * in main.c.
  ******************************************************************************
  *
  * main_blinky() creates one queue, one software timer, and two tasks.  It then
  * starts the scheduler.
  *
  * The Queue Send Task:
  * The queue send task is implemented by the prvQueueSendTask() function in
  * this file.  It uses vTaskDelayUntil() to create a periodic task that sends
  * the value 100 to the queue every 200 milliseconds (please read the notes
  * above regarding the accuracy of timing under Windows).
  *
  * The Queue Send Software Timer:
  * The timer is a one-shot timer that is reset by a key press.  The timer's
  * period is set to two seconds - if the timer expires then its callback
  * function writes the value 200 to the queue.  The callback function is
  * implemented by prvQueueSendTimerCallback() within this file.
  *
  * The Queue Receive Task:
  * The queue receive task is implemented by the prvQueueReceiveTask() function
  * in this file.  prvQueueReceiveTask() waits for data to arrive on the queue.
  * When data is received, the task checks the value of the data, then outputs a
  * message to indicate if the data came from the queue send task or the queue
  * send software timer.
  *
  * Expected Behaviour:
  * - The queue send task writes to the queue every 200ms, so every 200ms the
  *   queue receive task will output a message indicating that data was received
  *   on the queue from the queue send task.
  * - The queue send software timer has a period of two seconds, and is reset
  *   each time a key is pressed.  So if two seconds expire without a key being
  *   pressed then the queue receive task will output a message indicating that
  *   data was received on the queue from the queue send software timer.
  *
  * NOTE:  Console input and output relies on Windows system calls, which can
  * interfere with the execution of the FreeRTOS Windows port.  This demo only
  * uses Windows system call occasionally.  Heavier use of Windows system calls
  * can crash the port.
  */

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

/*-----------------------------------------------------------*/

// Declares all variables needed to simulate control panel accurately
// mainly line values, front-end type stuff
/* Dividing line between dbg control and system display */
#define DBG_SCRN_DIV_X          32

/* Rows on debug control screen */
#define DBG_SCRN_TIME_ROW       12
#define DBG_SCRN_FLOAT_ROW      22

/* Button locations */
#define DBG_SCRN_BTN_X          35
#define DBG_SCRN_BTN_Y          17
#define DBG_SCRN_BTN_WIDTH      7
#define DBG_SCRN_BTN_HEIGHT     2
#define DBG_SCRN_BTN_COLOR      GREEN
#define DBG_SCRN_BTN_BLINK_COLOR RED

/* System display */
#define DBG_SCRN_DISP_X         33
#define DBG_SCRN_DISP_Y         13
#define DBG_SCRN_DISP_WIDTH     20

/* System printer */
#define DBG_SCRN_PRNTR_X        57
#define DBG_SCRN_PRNTR_Y        5
#define DBG_SCRN_PRNTR_WIDTH    20
#define DBG_SCRN_PRNTR_HEIGHT   15

/* Bell display */
#define DBG_SCRN_BELL_X         43
#define DBG_SCRN_BELL_Y         5
#define DBG_SCRN_BELL_WIDTH     10
#define DBG_SCRN_BELL_HEIGHT    3

/* Line drawing characters for text mode */
#define LINE_HORIZ              196
#define LINE_VERT               179
#define LINE_CORNER_NW          218
#define LINE_CORNER_NE          191
#define LINE_CORNER_SE          217
#define LINE_CORNER_SW          192
#define LINE_T_W                195
#define LINE_T_N                194
#define LINE_T_E                180
#define LINE_T_S                193
#define LINE_CROSS              197

/* Scalers for FreeRTOS Simulation */
#define X_SIMULATION_SCALER 1

/* Static Data */

/* Data for displaying and getting buttons */
#define BUTTON_ROWS             3
#define BUTTON_COLUMNS          3

// Declares arrays of buttons
/*static char* p_chButtonText[BUTTON_ROWS][BUTTON_COLUMNS] =
{
    (" PRT ", " 1 ", " TIME "),
    (" HST ", " 2 ", NULLP),
    (" ALL ", " 3 ", " RST ")
};

static char a_chButtonKey[BUTTON_ROWS][BUTTON_COLUMNS] =
{
    {'P', '1', 'T'},
    {'H', '2', '\x00'},
    {'A', '3', 'R'}
};*/

/* Button the user pressed. */
static WORD wButton;

/* Printer state. */
/* Printed lines. */
static char aa_charPrinted
[DBG_SCRN_PRNTR_HEIGHT][DBG_SCRN_PRNTR_WIDTH + 1];

/* Printing a line now. */
static int iPrinting = 0;

/* Debug variables for reading the tank levels. */
/* Float levels. */
static int a_iTankLevels[COUNTOF_TANKS] =
{ 4000, 7200, 6400 };

/* Which tank the system asked about. NO_TANK means that
   the simulated float hardware is not reading. */
static int iTankToRead = NO_TANK;

/* Which tank the user is changing. */
static int iTankChanging = 0;

/* Is time passing automatically? */
static BOOL fAutoTime = FALSE;

/* Tasks and stacks for debugging */
#define STK_SIZE 1024
UWORD DebugKeyStk[STK_SIZE];
UWORD DebugTimerStk[STK_SIZE];

static void vDebugKeyTask(void* data);
static void vDebugTimerTask(void* data);
static SemaphoreHandle_t xWinSem;

/* Place to store DOS timer interrupt vector. */
//static void interrupt far(*OldTickISR)(void);

/* Static Functions */
static void vUtilityDrawBox(int ixNW, int iyNW, int iXSize, int iYSize);
//static void vUtilityDisplayFloatLevels(void);
//static void vUtilityPrinterDisplay(void);
static void gotoxy(int x, int y);

/*-----------------------------------------------------------*/

/*** SEE THE COMMENTS AT THE TOP OF THIS FILE ***/
void dbgmain(void)
{
    vUtilityDrawBox(5, 5, 10, 10);

    vHardwareInit();

    vTaskStartScheduler();


    for (;; );
}

void vHardwareInit(void) {
    int iColumn, iRow; /* Iterators */
    BYTE byErr; /* May not be necessary, used in micro c semaphore pending function*/

    /* Start the debugging tasks */
    xTaskCreate(vDebugTimerTask,"dbtimer", configMINIMAL_STACK_SIZE,NULL,TASK_PRIORITY_DEBUG_TIMER,NULL);

    xWinSem = xSemaphoreCreateBinary();

    configASSERT(xWinSem != NULL);



}

static void vDebugKeyTask(void* pvParameters){

    /* Prevent the compiler warning about the unused parameter. */
    (void)pvParameters;

    for (;;) {
        
    }

}

static void vDebugTimerTask(void* pvParameters){

    /* Prevent the compiler warning about the unused parameter. */
    (void)pvParameters;

    for (;;) {
        taskENTER_CRITICAL();
        printf("vDebugTimerTask\r\n");
        taskEXIT_CRITICAL();
        vTaskDelay(1000);
    }

}

/***** vUtilityDrawBox **********************************************

This routine draws a box.

RETURNS: None.
*/

static void vUtilityDrawBox(/*INPUTS:*/
    int iXNW,  /* x-coord of northwest corner of the box. */
    int iYNW,  /* y-coord of northwest corner of the box. */
    int iXSize, /* Inside width of the box. */
    int iYSize) /* Inside height of the box. */
{

    // Local Iterators
    int iColumn, iRow;

    // Personal adjustment, keep scaler normal until all code is correctly ported.
    int iScaledXSize = X_SIMULATION_SCALER * iXSize;

    // Draw the top of the box
    gotoxy(iXNW, iYNW);
    printf("%c", LINE_CORNER_NW);
    for (iColumn = 0; iColumn < iScaledXSize; iColumn++) {
        printf("%c", LINE_HORIZ);
    }
    printf("%c", LINE_CORNER_NE);

    // Draw the sides
    for (iRow = 1; iRow <= iYSize; iRow++) {
        gotoxy(iXNW, iYNW + iRow);
        printf("%c", LINE_VERT);
        gotoxy(iXNW + iScaledXSize + 1, iYNW + iRow);
        printf("%c", LINE_VERT);
    }

    // Draw the bottom
    gotoxy(iXNW, iYNW + iYSize + 1);
    printf("%c", LINE_CORNER_SW);
    for (iColumn = 0; iColumn < iScaledXSize; iColumn++) {
        printf("%c", LINE_HORIZ);
    }
    printf("%c", LINE_CORNER_SE);
}

static void gotoxy(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = { x, y};
    SetConsoleCursorPosition(hConsole, pos);
}

