
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"


#define DEFAULT_STACK_SIZE 2048
#define CDC_ITF_TX      1


int state(void) {
    if ay => 0.85 //and btn_fxn
        printf(".");
    if ax >= 0.85 //and btn_fxn
        printf("-");
    else
        printf("Not a clear state")  ;
}
