/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "stdlib.h"

// Blinking rate in milliseconds
#define BLINKING_RATE 800ms
#define SCALING_FACTOR                                                         \
  0.107143 //(1.2/11.2) is the equivalent resistance used to scale before the
           // data is fed to ADC
#define VREF 3.3
Ticker flipper;
DigitalOut led(LED1);
bool bTestEn = false;

DigitalOut CSEn(PB_4);
DigitalOut CS_ISet1(PF_12);
DigitalOut CS_ISet2(PD_15);
AnalogOut aout(PA_4);
AnalogIn PB1(PB_1);
AnalogIn PF4(PF_4);
AnalogIn PC2(PC_2);
Timer tCountTime;
void setCurrentmA(float fVal);
void startUp();
void legacyADV();
void UART_ADV_TRx();
void startADV();
std::chrono::milliseconds waitForCapToCharge(uint16_t nSamples,
                                             uint16_t reSample);
void charDelay();
float sampleADC(int);

void flip() { led = !led; }
int main() {

  printf("\n---Capacitor Bank Energy Characterization---\n");
  // Initialise the digital pin LED1 as an output
  CSEn = 0;
  CS_ISet1 = 0;
  CS_ISet2 = 0;
  setCurrentmA(0);

  /* Time to take average of 500 samples is 3 ms and
     at lowest current of 270uA, the ripple has a duration of
     900 ms and upon poweron from a blank state takes approx 30 sec*/
  std::chrono::milliseconds timeMS = waitForCapToCharge(50, 300);
  printf("Time to fullycharge is %llu\n", timeMS.count());
/*
  for (int i = 0; i < 5; i++)
    startUp();
*/
  while (1) {
    if (bTestEn) {
      printf("ADC Data PB1 - %05.3f PC2 - %05.3f PF4 - %05.3f\n",
             PB1.read() * 3.3, PC2.read() * 3.3, PF4.read() * 3.3);
      ThisThread::sleep_for(2000ms);
    }
  };
}

void startADV() {
  uint8_t iCount = 0;
  uint16_t advInterval = 160; // ms
  for (iCount = 0; iCount < 4; iCount++) {
    // Do UART
    UART_ADV_TRx();
    // wait
    ThisThread::sleep_for(27ms);
    // Adv
    legacyADV();
    // wait
    ThisThread::sleep_for((advInterval - 27) * 1ms);
  }

  // wait
  ThisThread::sleep_for(27ms);

  for (iCount = 0; iCount < 10; iCount++) {
    legacyADV();
    // wait
    ThisThread::sleep_for((advInterval * 1ms));
  }
}

void legacyADV() {
  setCurrentmA(4);
  ThisThread::sleep_for(5ms);
  setCurrentmA(0);
}

void UART_ADV_TRx() // using only the maximum of all
{
  setCurrentmA(2.3);
  ThisThread::sleep_for(63ms);
  setCurrentmA(0);
}
void startUp() {
  printf("\n1. Startup Routine \n");
  setCurrentmA(1.8);
  ThisThread::sleep_for(237ms);
  setCurrentmA(0);
  /* Sample duration 3ms and resample every 250ms */
  waitForCapToCharge(500, 250);
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

std::chrono::milliseconds waitForCapToCharge(uint16_t nSamples,
                                             uint16_t reSample) {

  std::chrono::milliseconds timeElapsedms;
  float fBankVoltage = 0;
  printf("Wait for Bank to charge to - 9.6V\n");
  fBankVoltage = PF4.read() * (VREF / SCALING_FACTOR);
  printf("At this time, Bank voltage is %f V\n", fBankVoltage);

  tCountTime.start();
  do {
    fBankVoltage = sampleADC(nSamples); // PF4.read() * (VREF / SCALING_FACTOR);
    ThisThread::sleep_for(reSample * 1ms);
    printf("voltage is %f V\n", fBankVoltage);
  } while (abs(fBankVoltage - 9.6) > 0.3);
  tCountTime.stop();
  timeElapsedms = std::chrono::duration_cast<std::chrono::milliseconds>(tCountTime.elapsed_time());
  tCountTime.reset();

  printf("Current charge level is %f V\n", fBankVoltage);
  printf("---Bank is now fully charged!!---\n");

  return timeElapsedms;
}

void charDelay() {
  tCountTime.start();
  ThisThread::sleep_for(25ms);
  tCountTime.stop();
  printf("The time taken was %llu milliseconds\n",
         std::chrono::duration_cast<std::chrono::milliseconds>(
             tCountTime.elapsed_time())
             .count());
  tCountTime.reset();
}

float sampleADC(int nSamples) {
  float fSample = 0;
  for (int i = 0; i < nSamples; i++) {
    fSample = fSample + PF4.read();
  }

  return (fSample / nSamples) * (VREF / SCALING_FACTOR);
}