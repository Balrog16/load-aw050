/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"


// Blinking rate in milliseconds
#define BLINKING_RATE     500ms
Ticker flipper;
DigitalOut led(LED1);

void flip()
{
    led = !led;
}

int main()
{
    // Initialise the digital pin LED1 as an output
   

    flipper.attach(&flip, 2s);

    while (true) {
        
       // ThisThread::sleep_for(BLINKING_RATE);
    }
}
