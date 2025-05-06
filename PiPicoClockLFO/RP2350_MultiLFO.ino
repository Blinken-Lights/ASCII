/*
* For Pi Pico 2
* CPU Speed: 250MHz (Overclock)
* Optimize: Optimize Even More (-O3)
*/
#include <SPI.h>
#include "Ucglib.h"
#include "pio_encoder.h"

#include <RPi_Pico_TimerInterrupt.h>
#include <RPi_Pico_ISR_Timer.h>
#include <RPi_Pico_ISR_Timer.hpp>

#include "wavetables.h"

Ucglib_SSD1351_18x128x128_HWSPI ucg(/*cd=*/21, /*cs=*/17, /*reset=*/20);

RPI_PICO_Timer ITimer0(0);

PioEncoder encoder(26);
byte encoderButtonPin = 22;
int encoderPosition = 0;
int backwardsEncoderDivider = 0;
int encoderPresses = 0;
bool prevEncoderButtonState = false;
int encoderButtonDebounce = 125;
int encoderButtonDoubleClick = 100;
unsigned long lastEncoderButtonPress = 0;

int knobValue = 0;
int jitterThreshold = 15;
int startChangingValueThreshold = 50;
bool changingValue = false;
int newViewValue = 0;
int knobPin = A2;

bool buttonState = false;
int buttonPin = 16;
unsigned long lastButtonPress = 0;
int buttonDebounce = 125;

int updateDisplayMs = 30;
unsigned long lastUpdatedDisplay = 0;

int gateOuts[] = { 8, 9, 10, 11 };
byte gateOn = 0;
unsigned long lastChangedGate = 0;
int changeGateEvery = 1000;

int dacCss[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
int dacs[] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7 };
int dacChannels[] = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };

float waveSpeeds[] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
float accumulators[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int waveTypes[16];
float amplitudes[16];
int waveDisplayAmplitudes[16];
int prevWaveDisplayAmplitudes[16];
int waveDisplayBuffers[16];
int waveDisplayPosition = 0;
int selectedWave = 0;
int fullSizeWaveDisplayPosition = 0;
int fullSizeWaveDisplayBuffer = 0;

int amplitudeModulators[16];
float amplitudeModulationAmounts[16];
int frequencyModulators[16];
float frequencyModulationAmounts[16];
int syncs[16];

int displayView = 0;
// 0 = wave overview
// 1 = changing wave

void setup(void) {

  SPI1.setRX(12);
  SPI1.setCS(13);
  SPI1.setSCK(14);
  SPI1.setTX(15);
  SPI1.begin(false);
  SPI1.beginTransaction(SPISettings(25000000, MSBFIRST, SPI_MODE0));

  for (byte i = 0; i < 8; i++) {
    pinMode(dacCss[i], OUTPUT);
  }

  for (byte i = 0; i < 4; i++) {
    pinMode(gateOuts[i], OUTPUT);
  }

  for (int i = 0; i < 16; i++) {
    amplitudes[i] = 1.0f;
    amplitudeModulators[i] = -1;
    amplitudeModulationAmounts[i] = 1.0f;
    frequencyModulators[i] = -1;
    frequencyModulationAmounts[i] = 1.0f;
    syncs[i] = -1;
  }

  ITimer0.attachInterruptInterval(100, TimerHandler0);  // 10000Hz
}

void setup1() {

  for (int i = 0; i < 16; i++) {
    waveDisplayBuffers[16] = 0;
  }

  encoder.begin();
  encoder.setMode(COUNT_1X);
  pinMode(encoderButtonPin, INPUT_PULLUP);

  analogReadResolution(12);
  knobValue = analogRead(knobPin);
  newViewValue = knobValue;

  SPI.setSCK(18);
  SPI.setTX(19);
  SPI.setRX(16);
  SPI.setCS(17);
  ucg.begin(UCG_FONT_MODE_SOLID);
  ucg.setFont(ucg_font_7x13_mr);
  ucg.setColor(0, 255, 255, 255);
  ucg.setColor(1, 0, 0, 0);
  ucg.clearScreen();

  pinMode(buttonPin, INPUT_PULLUP);
}

bool TimerHandler0(struct repeating_timer *t) {

  for (byte i = 0; i < 16; i++) {

    if (syncs[i] > -1 && prevWaveDisplayAmplitudes[syncs[i]] <= 0 && waveDisplayAmplitudes[syncs[i]] > 0) {
      accumulators[i] = 0;
    }

    int currentAccumulator = (int)round(accumulators[i]);
    if (currentAccumulator > 4095) {
      currentAccumulator = 4095;
    } else if (currentAccumulator < 0) {
      currentAccumulator = 0;
    }
    int newWaveAmplitude = (int)round(waveTables[waveTypes[i]][currentAccumulator] * amplitudes[i]);

    if (amplitudeModulators[i] > -1) {
      newWaveAmplitude = (int)round(newWaveAmplitude * ((float)waveDisplayAmplitudes[amplitudeModulators[i]] / 4095.0f)) * amplitudeModulationAmounts[i];
    }

    if (newWaveAmplitude > 4095) {
      newWaveAmplitude = 4095;
    } else if (newWaveAmplitude < 0) {
      newWaveAmplitude = 0;
    }

    setOutputChip1Channel(newWaveAmplitude, dacChannels[i], dacs[i]);

    prevWaveDisplayAmplitudes[i] = waveDisplayAmplitudes[i];
    waveDisplayAmplitudes[i] = newWaveAmplitude;

    float nextWaveIncrement = waveSpeeds[i];
    if (frequencyModulators[i] > -1) {
      nextWaveIncrement = nextWaveIncrement * ((float)waveDisplayAmplitudes[frequencyModulators[i]] / 2047.0f) * frequencyModulationAmounts[i];
    }

    accumulators[i] += nextWaveIncrement;
    if ((int)round(accumulators[i]) > 4095.0f) {
      accumulators[i] = 0;
    }
  }

  return true;
}

void loop(void) {

  if (millis() - lastChangedGate > changeGateEvery) {
    lastChangedGate = millis();
    for (int i = 0; i < 4; i++) {
      digitalWrite(gateOuts[i], gateOn == i);
    }
    gateOn++;
    if (gateOn >= 4) {
      gateOn = 0;
    }
  }
}

void loop1() {

  if (millis() - lastUpdatedDisplay >= updateDisplayMs) {
    lastUpdatedDisplay = millis();
    //ucg.setRotate90();
    ucg.setFont(ucg_font_ncenR12_tr);
    ucg.setColor(255, 255, 255);
    //ucg.setColor(0, 255, 0);
    //ucg.setColor(1, 255, 0,0);

    /*ucg.setPrintPos(0,25);
    ucg.print("Poopy McPoopface!");

    ucg.setPrintPos(0,40);
    ucg.print(encoderPosition);

    ucg.setPrintPos(0,60);
    ucg.print(encoderPresses);*/

    /*ucg.setPrintPos(0, 25);
    ucg.print((knobValue >> 4));
    ucg.setPrintPos(0,40);
    ucg.print(buttonState ? "Pressed" : "Released");*/

    /*ucg.setPrintPos(0, 25);
    ucg.print(displayView);
    ucg.setPrintPos(0,40);
    ucg.print(frequencyModulators[selectedWave]);*/

    if (displayView == 1) {
      displayFullSizeWave();
    } else {
      displayWaves(false);
    }
  }

  if (encoder.getCount()) {
    encoderChangedBy(encoder.getCount());
    encoder.reset();
  }

  bool encoderButtonState = !digitalRead(encoderButtonPin);
  if (encoderButtonState && !prevEncoderButtonState && millis() - lastEncoderButtonPress > encoderButtonDebounce) {
    encoderPresses++;
    lastEncoderButtonPress = millis();
    encoderPressed();
  } else if (encoderButtonState && !prevEncoderButtonState && millis() - lastEncoderButtonPress > encoderButtonDoubleClick) {
    encoderDoubleClick();
  }
  prevEncoderButtonState = encoderButtonState;

  int newKnobValue = analogRead(knobPin);
  if (abs(newKnobValue - knobValue) > jitterThreshold) {
    knobValue = newKnobValue;
    if(changingValue){
      knobValueChanged(knobValue);
    }
  }
  if(abs(newKnobValue - newViewValue) > startChangingValueThreshold) {
    newViewValue = newKnobValue;
    changingValue = true;
    knobValueChanged(knobValue);
  }

  bool buttonCurrentState = !digitalRead(buttonPin);
  if (buttonCurrentState && buttonState && millis() - lastButtonPress > buttonDebounce) {
    lastButtonPress = millis();
    buttonPressed();
  }
  buttonState = buttonCurrentState;

}

void encoderChangedBy(int difference) {

  changingValue = false; // stop potentiometer jitters changing settings within a new view

  // Hack because encoder counts 4x speed backwards
  if (difference < 0) {
    backwardsEncoderDivider++;
    if (backwardsEncoderDivider < 3) {
      return;
    } else {
      backwardsEncoderDivider = 0;
    }
  }
  difference = (difference > 0) ? 1 : -1;

  encoderPosition += difference;

  if (displayView == 0) {
    int newSelectedWave = selectedWave + difference;
    if (newSelectedWave >= 16) {
      newSelectedWave = 0;
    } else if (newSelectedWave < 0) {
      newSelectedWave = 15;
    }
    selectedWave = newSelectedWave;
  }/* else if (displayView == 1) {
    float newWaveSpeed = waveSpeeds[selectedWave] + difference;
    if (newWaveSpeed > 100.0f) {
      newWaveSpeed = 100.0f;
    } else if (newWaveSpeed < 0.01f) {
      newWaveSpeed = 0.01f;
    }
    waveSpeeds[selectedWave] = newWaveSpeed;
  }*/ else if (displayView == 1) {
    int newWaveType = waveTypes[selectedWave] + difference;
    if (newWaveType >= 7) {
      newWaveType = 0;
    } else if (newWaveType < 0) {
      newWaveType = 6;
    }
    waveTypes[selectedWave] = newWaveType;
  } else if (displayView == 2) {
    int newAmplitudeModulator = amplitudeModulators[selectedWave] + difference;
    if(newAmplitudeModulator == selectedWave){
      newAmplitudeModulator++;
    }
    if (newAmplitudeModulator >= 16) {
      newAmplitudeModulator = -1;
    } else if (newAmplitudeModulator < -1) {
      newAmplitudeModulator = 15;
    }
    amplitudeModulators[selectedWave] = newAmplitudeModulator;
  } else if (displayView == 3) {
    int newFrequencyModulator = frequencyModulators[selectedWave] + difference;
    if(newFrequencyModulator == selectedWave){
      newFrequencyModulator++;
    }
    if (newFrequencyModulator >= 16) {
      newFrequencyModulator = -1;
    } else if (newFrequencyModulator < -1) {
      newFrequencyModulator = 15;
    }
    frequencyModulators[selectedWave] = newFrequencyModulator;
  } else if (displayView == 4) {
    int newSync = syncs[selectedWave] + difference;
    if(newSync == selectedWave){
      newSync++;
    }
    if (newSync >= 16) {
      newSync = -1;
    } else if (newSync < -1) {
      newSync = 15;
    }
    syncs[selectedWave] = newSync;
  }
}

void encoderPressed() {

  changingValue = false; // stop potentiometer jitters changing settings within a new view

  /*if(displayView == 0){
    displayView = 1;
  }else if(displayView == 1){
    displayView = 2;
  }else if(displayView == 2){
    displayView = 0;
  }*/

  displayView++;
  if (displayView >= 5) {
    displayView = 0;
  }
}

void encoderDoubleClick() {

  changingValue = false; // stop potentiometer jitters changing settings within a new view

  //displayView = 0;

}

void buttonPressed() {

  if(displayView != 0){
    changingValue = false; // stop potentiometer jitters changing settings within a new view
    displayView = 0;
  }else{
    saveToEEPROM();
  }

}

void knobValueChanged(int value){

  if(displayView == 0){
    float newAmplitude = (float)value / 4095.0f;
    if (newAmplitude > 1.0f) {
      newAmplitude = 1.0f;
    } else if (newAmplitude < 0.0f) {
      newAmplitude = 0.0f;
    }
    amplitudes[selectedWave] = newAmplitude;
  }else if (displayView == 1) {
    float newWaveSpeed = (float)value / 3072.0f;
    if (newWaveSpeed > 100.0f) {
      newWaveSpeed = 100.0f;
    } else if (newWaveSpeed < 0.01f) {
      newWaveSpeed = 0.001f;
    }
    waveSpeeds[selectedWave] = newWaveSpeed;
  }else if(displayView == 2){
    float newAmplitudeModulationAmount = (float)value / 4095.0f;
    if (newAmplitudeModulationAmount > 1.0f) {
      newAmplitudeModulationAmount = 1.0f;
    } else if (newAmplitudeModulationAmount < 0.0f) {
      newAmplitudeModulationAmount = 0.0f;
    }
    amplitudeModulationAmounts[selectedWave] = newAmplitudeModulationAmount;
  }else if(displayView == 3){
    float newFrequencyModulationAmount = (float)value / 4095.0f;
    if (newFrequencyModulationAmount > 1.0f) {
      newFrequencyModulationAmount = 1.0f;
    } else if (newFrequencyModulationAmount < 0.0f) {
      newFrequencyModulationAmount = 0.0f;
    }
    frequencyModulationAmounts[selectedWave] = newFrequencyModulationAmount;
  }

}

void setOutputChip1Channel(uint16_t val, byte channel, byte chip) {
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | channel << 7 | (/*0x0*/ 0x1 << 5) | (1 << 4);

  digitalWrite(dacCss[chip], LOW);

  SPI1.transfer(highByte);
  SPI1.transfer(lowByte);

  digitalWrite(dacCss[chip], HIGH);
}

void displayWaves(bool halfSized) {

  int windowSize = halfSized ? 16 : 32;
  int scaleFactor = halfSized ? 8 : 7;
  int scanFactor = halfSized ? 2 : 1;

  for (int i = 0; i < 16; i++) {
    int waveAmplitude = waveDisplayAmplitudes[i] >> scaleFactor;
    int x = (i % 4) * windowSize;
    int y = round(i / 4) * windowSize;

    ucg.setColor(0, 0, 0);
    ucg.drawLine(x + (waveDisplayPosition / scanFactor) + 1, y + windowSize, x + (waveDisplayPosition / scanFactor) + 1, y);
    if (selectedWave == i) {
      ucg.setColor(255, 0, 0);
      int amplitudeBarLength = (windowSize - 2) * amplitudes[selectedWave];
      ucg.drawLine(x + 1, y + windowSize - 1, x + amplitudeBarLength, y + windowSize - 1);
      if (amplitudeBarLength < (windowSize - 2)) {
        ucg.setColor(0, 0, 0);
        ucg.drawLine(x + amplitudeBarLength + 2, y + windowSize - 1, x + windowSize - 1, y + windowSize - 1);
      }
      ucg.setColor(255, 0, 0);
    } else if (amplitudeModulators[selectedWave] == i) {
      ucg.setColor(0, 255, 0);
      int amplitudeModulationBarLength = (windowSize - 2) * amplitudeModulationAmounts[selectedWave];
      ucg.drawLine(x + 1, y + windowSize - 1, x + amplitudeModulationBarLength, y + windowSize - 1);
      if (amplitudeModulationBarLength < (windowSize - 2)) {
        ucg.setColor(0, 0, 0);
        ucg.drawLine(x + amplitudeModulationBarLength + 2, y + windowSize - 1, x + windowSize - 1, y + windowSize - 1);
      }
      ucg.setColor(0, 255, 0);
    } else if (frequencyModulators[selectedWave] == i) {
      ucg.setColor(0, 0, 255);
      int frequencyModulationBarLength = (windowSize - 2) * frequencyModulationAmounts[selectedWave];
      ucg.drawLine(x + 1, y + windowSize - 1, x + frequencyModulationBarLength, y + windowSize - 1);
      if (frequencyModulationBarLength < (windowSize - 2)) {
        ucg.setColor(0, 0, 0);
        ucg.drawLine(x + frequencyModulationBarLength + 2, y + windowSize - 1, x + windowSize - 1, y + windowSize - 1);
      }
      ucg.setColor(0, 0, 255);
    } else if (syncs[selectedWave] == i) {
      ucg.setColor(255, 0, 255);
    } else {
      ucg.setColor(255, 255, 255);
    }
    ucg.drawLine(x + (waveDisplayPosition / scanFactor), y + windowSize - waveDisplayBuffers[i], x + (waveDisplayPosition / scanFactor) + 1, y + windowSize - waveAmplitude);

    waveDisplayBuffers[i] = waveAmplitude;
  }

  if(displayView == 1){
    ucg.setColor(255, 0, 0);
  }else if(displayView == 2){
    ucg.setColor(0, 255, 0);
  }else if(displayView == 3){
    ucg.setColor(0, 0, 255);
  }else if(displayView == 4){
    ucg.setColor(255, 0, 255);
  }else{
    ucg.setColor(255, 255, 255);
  }
  for (int i = 1; i < 4; i++) {
    ucg.drawLine(0, i * windowSize, 127, i * windowSize);
  }
  for (int i = 1; i < 4; i++) {
    ucg.drawLine(i * windowSize, 0, i * windowSize, 127);
  }

  waveDisplayPosition++;
  if (waveDisplayPosition >= 32) {
    waveDisplayPosition = 0;
  }
}

void displayFullSizeWave() {

  ucg.setColor(0, 0, 0);
  ucg.drawLine(fullSizeWaveDisplayPosition + 1, 127, fullSizeWaveDisplayPosition + 1, 0);

  int waveAmplitude = waveDisplayAmplitudes[selectedWave] >> 5;

  ucg.setColor(255, 0, 0);
  ucg.drawLine(fullSizeWaveDisplayPosition, 127 - fullSizeWaveDisplayBuffer, fullSizeWaveDisplayPosition + 1, 127 - waveAmplitude);

  fullSizeWaveDisplayBuffer = waveAmplitude;

  fullSizeWaveDisplayPosition++;
  if (fullSizeWaveDisplayPosition >= 128) {
    fullSizeWaveDisplayPosition = 0;
  }
}

void saveToEEPROM(){

}