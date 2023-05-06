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
void flip() { led = !led; }

int main() {
  // Initialise the digital pin LED1 as an output

  flipper.attach(&flip, 2s);

  while (true) {

    for (float i = 0.0f; i < 1.0f; i += 0.1f) {
      aout = i;
      ThisThread::sleep_for(BLINKING_RATE);
    }
  }
}
