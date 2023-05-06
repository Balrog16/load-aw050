/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

// Blinking rate in milliseconds
#define BLINKING_RATE 800ms
Ticker flipper;
DigitalOut led(LED1);
AnalogOut aout(PA_4);

void flip() { led = !led; }


void setCurrentmA(float fVal)
{
    //if fVal is zero .. put it to Sleep

    float m=0, c=0;

    if(fVal>0 && fVal<0.35) fVal = 0.35;
    if(fVal>2.7 && fVal<3) fVal = 3;
    if(fVal>5.13 && fVal<=5.3) fVal = 5.3;
    if(fVal>=0.35 && fVal<=2.7)
    {
        m = 13.3839;
        c = 0.0199;
    }
    else
    {
        if(fVal>=3 && fVal<=5.13)
        {
            m = 13.9429;
            c = -0.1043;
        }

        if(fVal>=5.3 && fVal<=9.2)
        {
            m = 12.7761;
            c = 0.1733;
        }
        if(fVal>9.2 && fVal<=12)
        {
            m = 12.3;
            c = 0.55;
        }
    }

    float setmA = (fVal - c)/m;
    aout = setmA;
}




int main() {
  // Initialise the digital pin LED1 as an output

 

  while (true) {

   for (float i = 0.1f; i < 12.0f; i += 0.5f) {
      setCurrentmA(i);
      ThisThread::sleep_for(BLINKING_RATE);}

  }
}
