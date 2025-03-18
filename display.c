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