/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

// Blinking rate in milliseconds
#define BLINKING_RATE 1500ms
Ticker flipper;
DigitalOut led(LED1);
AnalogOut aout(PA_4);
float i = 0;
void flip() { led = !led; }

void testDAC()
{
  /*aout = 0.3;
  i = i + 0.1;
  if(i == 1.0f) i=0;*/
}

int main() {
  // Initialise the digital pin LED1 as an output

  flipper.attach(&testDAC, 2s);

  while (true) {

    for (float i = 0.0f; i < 1.0f; i += 0.1f) {
      aout = i;
      ThisThread::sleep_for(BLINKING_RATE);}

  }
}
