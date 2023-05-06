/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

// Blinking rate in milliseconds
#define BLINKING_RATE 800ms
Ticker flipper;
DigitalOut led(LED1);

DigitalOut CSEn(PB_4);
DigitalOut CS_ISet1(PF_12);
DigitalOut CS_ISet2(PD_15);
AnalogOut aout(PA_4);

void setCurrentmA(float fVal);

void flip() { led = !led; }

int main() {
  // Initialise the digital pin LED1 as an output
  CSEn = 0;
  CS_ISet1 =0;
  CS_ISet2 = 0;

  // Put it in sleep to begin with
  CSEn = 1;
  aout = 0;
  //setCurrentmA(0);
  ThisThread::sleep_for(3000ms);

  CSEn = 0;
  //aout = 0.3;
  //setCurrentmA(4);
  setCurrentmA(0.4);
  ThisThread::sleep_for(3000ms);
  CSEn = 0;
  setCurrentmA(6.6);
    ThisThread::sleep_for(3000ms);

  while (true) {

    for (float i = 0.39; i < 12.0; i += 0.3) {
      setCurrentmA(i);
      ThisThread::sleep_for(BLINKING_RATE);
    }
  }
}

void setCurrentmA(float fVal) {

  float m = 0, c = 0;

  if (fVal > 0 && fVal < 0.35)
    fVal = 0.35;
  if (fVal > 2.7 && fVal <= 3)
    fVal = 3;
  if (fVal > 5.13 && fVal <= 5.3)
    fVal = 5.3;
  if (fVal >= 0.35 && fVal <= 2.7) {
    m = 13.3839;
    c = 0.0199;
  } else {
    if (fVal >= 3 && fVal <= 5.13) {
      m = 13.9429;
      c = -0.1043;
    }

    if (fVal >= 5.3 && fVal <= 9.2) {
      m = 12.7761;
      c = 0.1733;
    }
    if (fVal > 9.2 && fVal <= 12) {
      m = 12.3;
      c = 0.55;
    }
  }

  // if fVal is zero .. put it to Sleep
  if (fVal == 0) {
    CSEn = 1;
    m = 1.0;
    c = 0.0;
  } else
    CSEn = 0;

  float setmA = (fVal - c) / m;
  aout = setmA;
}
