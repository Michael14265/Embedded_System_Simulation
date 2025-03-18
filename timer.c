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

