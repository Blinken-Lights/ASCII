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

Ucglib_SSD1351_18x128x128_HWSPI ucg(/*cd=*/ 21, /*cs=*/ 17, /*reset=*/ 20);

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
int waveDisplayAmplitudes[16];
int prevWaveDisplayAmplitudes[16];
int waveDisplayBuffers[16];
int waveDisplayPosition = 0;
int selectedWave = 0;

int amplitudeModulators[16];
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

  for(byte i = 0; i < 8; i++){
    pinMode(dacCss[i], OUTPUT);
  }

  for(byte i = 0; i < 4; i++){
    pinMode(gateOuts[i], OUTPUT);
  }

  for(int i = 0; i < 16; i++){
    amplitudeModulators[i] = -1;
    syncs[i] = -1;
  }

  ITimer0.attachInterruptInterval(100, TimerHandler0); // 10000Hz

}

void setup1(){

  for(int i = 0; i < 16; i++){
    waveDisplayBuffers[16] = 0;
  }

  encoder.begin();
  encoder.setMode(COUNT_1X);
  pinMode(encoderButtonPin, INPUT_PULLUP);

  SPI.setSCK(18);
  SPI.setTX(19);
  SPI.setRX(16);
  SPI.setCS(17);
  ucg.begin(UCG_FONT_MODE_SOLID);
  ucg.setFont(ucg_font_7x13_mr);
  ucg.setColor(0, 255, 255, 255);
  ucg.setColor(1, 0, 0, 0);
  ucg.clearScreen();

}

bool TimerHandler0(struct repeating_timer *t)
{

  for(byte i = 0; i < 16; i++){

    if(syncs[i] > -1 && prevWaveDisplayAmplitudes[syncs[i]] <= 0 && waveDisplayAmplitudes[syncs[i]] > 0){
      accumulators[i] = 0;
    }

    int currentAccumulator = (int)round(accumulators[i]);
    if(currentAccumulator > 4095){
      currentAccumulator = 4095;
    }else if(currentAccumulator < 0){
      currentAccumulator = 0;
    }
    int newWaveAmplitude = waveTables[waveTypes[i]][currentAccumulator];

    if(amplitudeModulators[i] > -1){
      newWaveAmplitude = (int)round(newWaveAmplitude * ((float)waveDisplayAmplitudes[amplitudeModulators[i]] / 4095.0f));
    }

    if(newWaveAmplitude > 4095){
      newWaveAmplitude = 4095;
    }else if(newWaveAmplitude < 0){
      newWaveAmplitude = 0;
    }

    setOutputChip1Channel(newWaveAmplitude, dacChannels[i], dacs[i]);

    prevWaveDisplayAmplitudes[i] = waveDisplayAmplitudes[i];
    waveDisplayAmplitudes[i] = newWaveAmplitude;

    accumulators[i]+= waveSpeeds[i];
    if((int)round(accumulators[i]) > 4095.0f){
      accumulators[i] = 0;
    }

  }

  return true;
}

void loop(void) {

  if(millis() - lastChangedGate > changeGateEvery){
    lastChangedGate = millis();
    for(int i = 0; i < 4; i++){
      digitalWrite(gateOuts[i], gateOn == i);
    }
    gateOn++;
    if(gateOn >= 4){
      gateOn = 0;
    }
  }

}

void loop1(){

  if(millis() - lastUpdatedDisplay >= updateDisplayMs){
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

    //if(displayView == 0 || displayView == 1){
      displayWaves(false);
    /*}else if(displayView == 2){
      displayWaves(true);
    }*/
  }

  if(encoder.getCount()){
    encoderChangedBy(encoder.getCount());
    encoder.reset();
  }

  bool encoderButtonState = !digitalRead(encoderButtonPin);
  if(encoderButtonState && !prevEncoderButtonState && millis() - lastEncoderButtonPress > encoderButtonDebounce){
    encoderPresses++;
    lastEncoderButtonPress = millis();
    encoderPressed();
  }else if(encoderButtonState && !prevEncoderButtonState && millis() - lastEncoderButtonPress > encoderButtonDoubleClick){
    encoderDoubleClick();
  }
  prevEncoderButtonState = encoderButtonState;

}

void encoderChangedBy(int difference){

  // Hack because encoder counts 4x speed backwards
  if(difference < 0){
    backwardsEncoderDivider++;
    if(backwardsEncoderDivider < 3){
      return;
    }else{
      backwardsEncoderDivider = 0;
    }
  }
  difference = (difference > 0) ? 1 : -1;

  encoderPosition+= difference;

  if(displayView == 0){
    int newSelectedWave = selectedWave + difference;
    if(newSelectedWave >= 16){
      newSelectedWave = 0;
    }else if(newSelectedWave < 0){
      newSelectedWave = 15;
    }
    selectedWave = newSelectedWave;
  }else if(displayView == 1){
    float newWaveSpeed = waveSpeeds[selectedWave] + difference;
    if(newWaveSpeed > 100.0f){
      newWaveSpeed = 100.0f;
    }else if(newWaveSpeed < 0.01f){
      newWaveSpeed = 0.01f;
    }
    waveSpeeds[selectedWave] = newWaveSpeed;
  }else if(displayView == 2){
    int newWaveType = waveTypes[selectedWave] + difference;
    if(newWaveType >= 7){
      newWaveType = 0;
    }else if(newWaveType < 0){
      newWaveType = 6;
    }
    waveTypes[selectedWave] = newWaveType;
  }else if(displayView == 3){
    int newAmplitudeModulator = amplitudeModulators[selectedWave] + difference;
    if(newAmplitudeModulator >= 16){
      newAmplitudeModulator = -1;
    }else if(newAmplitudeModulator < -1){
      newAmplitudeModulator = 15;
    }
    amplitudeModulators[selectedWave] = newAmplitudeModulator;
  }else if(displayView == 4){
    int newSync = syncs[selectedWave] + difference;
    if(newSync >= 16){
      newSync = -1;
    }else if(newSync < -1){
      newSync = 15;
    }
    syncs[selectedWave] = newSync;
  }

}

void encoderPressed(){

  /*if(displayView == 0){
    displayView = 1;
  }else if(displayView == 1){
    displayView = 2;
  }else if(displayView == 2){
    displayView = 0;
  }*/

  displayView++;
  if(displayView >= 5){
    displayView = 0;
  }

}

void encoderDoubleClick(){

  displayView = 0;

}

void setOutputChip1Channel(uint16_t val, byte channel, byte chip){
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | channel << 7 | (/*0x0*/0x1 << 5) | (1 << 4);

  digitalWrite(dacCss[chip], LOW);

  SPI1.transfer(highByte);
  SPI1.transfer(lowByte);

  digitalWrite(dacCss[chip], HIGH);
}

void displayWaves(bool halfSized){

  int windowSize = halfSized ? 16 : 32;
  int scaleFactor = halfSized ? 8 : 7;
  int scanFactor = halfSized ? 2 : 1;

  for(int i = 0; i < 16; i++){
    int waveAmplitude = waveDisplayAmplitudes[i] >> scaleFactor;
    int x = (i % 4) * windowSize;
    int y = round(i / 4) * windowSize;

    ucg.setColor(0, 0, 0);
    ucg.drawLine( x + (waveDisplayPosition / scanFactor) + 1, y + windowSize, x + (waveDisplayPosition / scanFactor) + 1, y );
    if(selectedWave == i){
      ucg.setColor(255, 0, 0);
    }else if(amplitudeModulators[selectedWave] == i){
      ucg.setColor(0, 255, 0);
    }else if(syncs[selectedWave] == i){
      ucg.setColor(0, 0, 255);
    }else{
      ucg.setColor(255, 255, 255);
    }
    ucg.drawLine( x + (waveDisplayPosition / scanFactor), y + windowSize - waveDisplayBuffers[i], x + (waveDisplayPosition / scanFactor) + 1, y + windowSize - waveAmplitude );

    waveDisplayBuffers[i] = waveAmplitude;
  }

  ucg.setColor(255, 255, 255);
  for(int i = 1; i < 4; i++){
    ucg.drawLine( 0, i * windowSize, 127, i * windowSize );
  }
  for(int i = 1; i < 4; i++){
    ucg.drawLine( i * windowSize, 0, i * windowSize, 127 );
  }

  waveDisplayPosition++;
  if(waveDisplayPosition >= 32){
    waveDisplayPosition = 0;
  }
}