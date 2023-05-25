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
bool bRun = false;

InterruptIn ctrlBtn(PC_13);
DigitalOut CSEn(PB_4);
DigitalOut CS_ISet1(PF_12);
DigitalOut CS_ISet2(PD_15);

DigitalOut dout(PA_4, 0);
AnalogIn PB1(PB_1);
AnalogIn PF4(PF_4);
AnalogIn PC2(PC_2);
void echoProfile();
Timer tCountTime;
uint8_t btnCnt;
void setCurrentmA(float fVal);
std::chrono::milliseconds startUp();
void legacyADV();
void UART_ADV_TRx();
void btnPress();
void makeConnection();
void readRegisters();
void signOfLife();
std::chrono::milliseconds waitForCapToCharge(uint16_t nSamples,
                                             uint16_t reSample, float fThreshV);
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
  ctrlBtn.rise(&btnPress);

  /* Time to take average of 500 samples is 3 ms and
     at lowest current of 270uA, the ripple has a duration of
     900 ms and upon poweron from a blank state takes approx 30 sec*/
  std::chrono::milliseconds timeMS = waitForCapToCharge(50, 300, 6.6);
  printf("Time to charge till 6.6V is %llu\n", timeMS.count());

  /* Check startup load */
  for (int i = 0; i < 1; i++) {

    timeMS = startUp();
    printf("Time to fully charge is %llu\n", timeMS.count());
  }

  /* introduce a sign of life advertisement at a low pace */
  // measure and if dropped below some thing then wait for it to raise
  float fBankVoltage = 0;
  do {
    // fBankVoltage = sampleADC(10);
    signOfLife();
  } while (1); // abs(fBankVoltage - 9.6) > 0.3 && (fBankVoltage < 9.6));

  if (bRun) {
    printf("Try UART\n");
    for (int i = 0; i < 4; i++)
      UART_ADV_TRx();

    printf("Start Legacy ADV for 15000 times\n");
    /* Check legacy ADV load */
    for (int i = 0; i < 15000; i++) {
      legacyADV();
      if (btnCnt == 1)
        break;
    }
    printf("ADV stopped to make connection\n");
    makeConnection();
    printf("Connection now moves to Trx\n");
    readRegisters();
    waitForCapToCharge(50, 300, 9.6);
    printf("move to echo\n");
    echoProfile();
  }
  while (1) {
    if (bTestEn) {
      printf("ADC Data PB1 - %05.3f PC2 - %05.3f PF4 - %05.3f\n",
             PB1.read() * 3.3, PC2.read() * 3.3, PF4.read() * 3.3);
      ThisThread::sleep_for(2000ms);
    }
  };
}

void makeConnection() {
  /* Connection and Bonding */
  float currents[] = {3.5, 0, 5.1, 0, 3.6, 0, 3.2, 0, 2.9, 0, 3, 0, 3, 0, 2.6};
  uint8_t duration[] = {6, 10, 8, 18, 3, 21, 2, 22, 27, 20, 3, 22, 3, 22, 17};

  for (int i = 0; i < 15; i++) {
    setCurrentmA(currents[i]);
    ThisThread::sleep_for(duration[i] * 1ms);
  }

  while (btnCnt != 2) {
    setCurrentmA(3.5);
    ThisThread::sleep_for(2ms);
    setCurrentmA(0);
    ThisThread::sleep_for(350ms);
  }
  setCurrentmA(0);
}

void echoProfile() {
  /* Connection and Bonding */
  float currents[] = {3, 0, 2, 0, 0, 3.3, 0, 0};
  uint8_t duration[] = {4, 6, 50, 240, 50, 2, 250, 100};

  int i = 5;
  while (btnCnt != 4) {
    /* Check the level of the bank */
    float fBankVoltage = 0;
    fBankVoltage = PF4.read() * (VREF / SCALING_FACTOR);

    if (i == 0 && fBankVoltage < 6.0) {
      // simple handshake

      for (i = 5; i < 8; i++) {
        setCurrentmA(currents[i]);
        ThisThread::sleep_for(duration[i] * 1ms);
      }
      i = 0;

    } else {
      if (i == 0) {
        // complete from i=2 downto 8
        for (i = 0; i < 4; i++) {
          setCurrentmA(currents[i]);
          ThisThread::sleep_for(duration[i] * 1ms);
        }
      } else {
        // handshake time
        for (i = 5; i < 8; i++) {
          setCurrentmA(currents[i]);
          ThisThread::sleep_for(duration[i] * 1ms);
        }
        i = 0;
      }
    }
  }

  setCurrentmA(0);
}

void readRegisters() {
  float currents[] = {3.3, 0, 3.5, 0, 2, 0, 2, 0};
  uint16_t duration[] = {2, 448, 3, 7, 2, 1, 124, 313};
  int i = 0;
  while (btnCnt != 3) {
    /* Check the level of the bank */
    float fBankVoltage = 0;
    fBankVoltage = PF4.read() * (VREF / SCALING_FACTOR);

    if (i == 2 && fBankVoltage < 6.0) {
      // simple handshake
      i = 0;
      for (i = 0; i < 2; i++) {
        setCurrentmA(currents[i]);
        ThisThread::sleep_for(duration[i] * 1ms);
      }

    } else {
      if (i == 2) {
        // complete from i=2 downto 8
        for (i = 2; i < 8; i++) {
          setCurrentmA(currents[i]);
          ThisThread::sleep_for(duration[i] * 1ms);
        }
      } else {
        // handshake time
        for (i = 0; i < 2; i++) {
          setCurrentmA(currents[i]);
          ThisThread::sleep_for(duration[i] * 1ms);
        }
      }
    }
    if (i == 8)
      i = 0;
  }
}

void legacyADV() {
  /* Legacy Adv */
  setCurrentmA(5);
  ThisThread::sleep_for(5ms);
  setCurrentmA(0);
  ThisThread::sleep_for(120ms);
  /* Timer to change the adv packet */
  setCurrentmA(1);
  ThisThread::sleep_for(2ms);
  setCurrentmA(0);
  ThisThread::sleep_for(40ms);
  // measure and if dropped below some thing then wait for it to raise
  float fBankVoltage = 0;
  fBankVoltage = PF4.read() * (VREF / SCALING_FACTOR);
  if (fBankVoltage < 6.0)
    waitForCapToCharge(5, 5, 7);
}

void UART_ADV_TRx() // using only the maximum of all
{
  setCurrentmA(2.3);
  ThisThread::sleep_for(63ms);
  setCurrentmA(0);
  waitForCapToCharge(5, 100, 6);
}
std::chrono::milliseconds startUp() {
  printf("\n1. Startup Routine \n");
  setCurrentmA(32);
  ThisThread::sleep_for(1ms);
  setCurrentmA(3);
  ThisThread::sleep_for(23ms);
  setCurrentmA(1.8);
  ThisThread::sleep_for(226ms);
  setCurrentmA(0);
  /* Sample duration 3ms and resample every 250ms */
  return waitForCapToCharge(50, 50, 6.6);
}

std::chrono::milliseconds
waitForCapToCharge(uint16_t nSamples, uint16_t reSample, float fThreshV) {

  std::chrono::milliseconds timeElapsedms;
  float fBankVoltage = 0;
  // printf("Wait for Bank to charge to - %f V\n", fThreshV);
  fBankVoltage = PF4.read() * (VREF / SCALING_FACTOR);
  // printf("At this time, Bank voltage is %f V\n", fBankVoltage);

  tCountTime.start();
  do {
    fBankVoltage = sampleADC(nSamples); // PF4.read() * (VREF / SCALING_FACTOR);
    ThisThread::sleep_for(reSample * 1ms);
    // printf("voltage is %f V\n", fBankVoltage);
  } while (abs(fBankVoltage - fThreshV) > 0.3 && (fBankVoltage < fThreshV));
  tCountTime.stop();
  timeElapsedms = std::chrono::duration_cast<std::chrono::milliseconds>(
      tCountTime.elapsed_time());
  tCountTime.reset();

  // printf("Current charge level is %f V\n", fBankVoltage);
  // printf("---Bank is now fully charged!!---\n");

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

void setCurrentmA(float fVal) {

  AnalogOut aout(PA_4);
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
    CS_ISet1 = 1;
    m = 1.0;
    c = 0.0;
  } else
  {
    CSEn = 0;
    CS_ISet1 =
  }
  float setmA = (fVal - c) / m;
  aout = setmA;
}

void btnPress() {
  ctrlBtn.rise(NULL);
  wait_us(1000000);
  btnCnt++;
  if (btnCnt == 5)
    btnCnt = 0;
  ctrlBtn.rise(&btnPress);
}

void signOfLife() {
  /* Legacy Adv only for sign of life*/
  setCurrentmA(5);
  ThisThread::sleep_for(5ms);
  setCurrentmA(0);
  ThisThread::sleep_for(750ms);
}