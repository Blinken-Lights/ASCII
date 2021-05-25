//MicroScope III

#include <SPI.h>
#include "Ucglib.h"
#include <STM32ADC.h>

STM32ADC myADC(ADC1);
uint16_t dataPoints[4];

Ucglib_SSD1351_18x128x128_HWSPI ucg(/*cd=*/ PB0, /*cs=*/ PA4, /*reset=*/ PB1);

//Ucglib_ST7735_18x128x160_HWSPI ucg(/*cd=*/ PB0, /*cs=*/ PA4, /*reset=*/ PB1);

/* tested working on chinese STM3

   compile with upload method ST Link chosen
   upload with:
   st-flash --debug --reset write [compiled binary location] 0x8000000

  jumpers both set to 0
*/

int h = 0;
// replace encoders with up/down buttons?
uint8 sensorPins[] = { PA0,  PA1,  PA2,  PA3 }; //{ PA0,  PA2,  PA6,  PB1 };
int a[4][512];
byte pa[4][128];
byte color[4][3] = {
  { 255,    0,    0 },
  { 0,    255,    0 },
  { 0,      0,  255 },
  { 255,  255,    0 },
};
int zero_offsets[] = { 2048, 2048, 2048, 2048 }; // +2048 - -2048;
int offsets[] = { 0, 0, 0, 0 }; // +2048 - -2048;
bool remove_dc[] = { true, true, true, true };
//bool remove_dc[] = { false, false, false, false };
float dc[] = { 0.0, 0.0, 0.0, 0.0 };
float dc_filter = 0.003;
float lp[] = { 0.0, 0.0, 0.0, 0.0 };
float lp_filter = 0.35;
int t = 64; // 7 bit int
int tf = 0;
int display_t = 64; // remember last displayed threshold, so it can be covered with black before user change takes effect

int frames = 0;
int refresh_bg = 30;

int sm = 6;
int fm = 3;
int tm = 1;
int bm = 8;

int set_channels = 4;
int cs = 4;

float max_d = 64.0;
float max_v = 5.4;
int sf = 1; // scale factor, minimum 1!
int sf_adj[7] = { 0, 0, 64, 192, 448, 960, 1984 };

//int tb = 9; // 9 microseconds minimum
//int tb = 10;
int tb = 10;

//buttons
int debounce = 200;
int buttons[] = { PB12, PB13, PB14, PB15, PA8 };
unsigned long last_read[] = { 0, 0, 0, 0, 0 };
int presses[] = { 0, 0, 0, 0, 0 };
int dpresses[] = { 0, 0, 0, 0, 0 };
unsigned long held[] = { 0, 0, 0, 0, 0 };

int sample = 0;
bool disp = false;

void setup(void)
{
  delay(1000);
  //ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.begin(UCG_FONT_MODE_SOLID);
  ucg.setFont(ucg_font_04b_03b_hr);
  ucg.clearScreen();

  SPI.setClockDivider(SPI_CLOCK_DIV4);

  myADC.calibrate();
  for (int i = 0; i < 4; i++) {
    pinMode(sensorPins[i], INPUT_ANALOG);
  }

  adc_set_prescaler(ADC_PRE_PCLK2_DIV_2);
  myADC.setSampleRate(ADC_SMPR_7_5);//set the Sample Rate (try setting dynamically for different timebases) - max usable frequency at ADC_SMPR_7_5 is up to 27kHz and much less crosstalk than 1.5 cycles
  myADC.setScanMode();              //set the ADC in Scan mode. 
  myADC.setPins(sensorPins, 4);     //set how many and which pins to convert.
  myADC.setContinuous();            //set the ADC in continuous mode.
  myADC.setDMA(dataPoints, 4, (DMA_MINC_MODE | DMA_CIRC_MODE), NULL);
  myADC.startConversion();

  for(int i = 0; i < 5; i++){
    pinMode(buttons[i], INPUT_PULLUP);   
  }
  attachInterrupt(digitalPinToInterrupt(buttons[0]),button0,FALLING);
  attachInterrupt(digitalPinToInterrupt(buttons[1]),button1,FALLING);
  attachInterrupt(digitalPinToInterrupt(buttons[2]),button2,FALLING);
  attachInterrupt(digitalPinToInterrupt(buttons[3]),button3,FALLING);
  attachInterrupt(digitalPinToInterrupt(buttons[4]),button4,FALLING);

  Timer2.setMode(TIMER_CH1, TIMER_OUTPUTCOMPARE);
  Timer2.setPeriod(tb); // in microseconds
  Timer2.setCompare(TIMER_CH1, 1);      // overflow might be small
  Timer2.attachInterrupt(TIMER_CH1, handler_sample);

  setup_scale(false, false);
  setup_scale(true, false);
}

void handler_sample(){
  if(disp){
    return;
  }
  
  for (int i = 0; i < cs; i++) {
    a[i][sample] = dataPoints[i];
  }
  
  sample++;
  if(sample >= 511){
    sample = 0;
    disp = true;
  }
}

void button0(){
  bpress(0);
}
void button1(){
  bpress(1);
}
void button2(){
  bpress(2);
}
void button3(){
  bpress(3);
}
void button4(){
  bpress(4);
}
void bpress(int i){
  unsigned long n = millis();
  if(n - last_read[i] < debounce){
    return;
  }
  last_read[i] = n;
  held[i] = n;

  presses[i]++;
}

void loop(void)
{

  // can do some things outside of waveform display

  for(int i=0; i<5; i++){
    unsigned long n = millis();
    bool state = digitalRead(buttons[i]);
    if(!state){
      int held_for = n - held[i];
      if(held_for > debounce){
        presses[i]++;
        held[i] = n;
      }
    }
    
    if(dpresses[i] != presses[i]){

      //if not shift / entered other menus
      if(i==0 || i==1){
        tb+= ((i==0)?1:-1);
        if(tb < 3){
          tb = 3;
        }
        ucg.clearScreen(); // clear old scale
        Timer2.setPeriod(tb);
        setup_scale(true, false); // draw new scale
        setup_scale(false, false);
      }else if(i==2 || i==3){
        sf+= ((i==2)?1:-1);
        sf = min(6,max(1,sf));
        ucg.clearScreen(); // clear old scale
        setup_scale(true, false); // draw new scale
        setup_scale(false, false);
      }
      
      dpresses[i] = presses[i];
    }
  }

  if(!disp){
    return;
  }

  bool threshold_found = false;

  // move all maths out of the sampling loop
  for (int x = 0; x < 512; x++) {
    for (int i = 0; i < cs; i++) {



      /*
      //try this to reduce noise:
      //if this works, change analog resolution to 10 bit and adjust other maths
      a[i][x] = (a[i][x] >> 2) << 2;*/

      
      a[i][x]-=zero_offsets[i];
      dc[i] = (dc_filter * a[i][x]) + ((1.0-dc_filter)*dc[i]);
      
      a[i][x] = ((((a[i][x] + offsets[i] - (remove_dc[i]?(int)dc[i]:0)) + 2048) >> (6 - sf)) - sf_adj[sf]);


      

      /*a[i][x] = ((a[i][x] + offsets[i]) >> (6 - sf)) - sf_adj[sf];

      a[i][x]-=64;
      dc[i] = (dc_filter * a[i][x]) + ((1.0-dc_filter)*dc[i]);
      if(remove_dc[i]){
        a[i][x]-=dc[i];
      }
      a[i][x]+=64;*/


      /*
      //apply lp filter after rescaling, so it should look ok at any resolution
      a[i][x]-=64;
      lp[i] = (lp_filter * a[i][x]) + ((1.0-lp_filter)*lp[i]);
      a[i][x] = lp[i] + 64;*/

      if (i == 0 && x > 64 && x < 448 // 448 = 512 - 64
          && !threshold_found && a[i][x] <= t
          && a[i][x - 1] > t) {
        tf = x;
        threshold_found = true;
      }
    }
  }

  if(frames > refresh_bg){
    setup_scale(true, false);
    frames = 0;
  }

  ucg.setColor(0, 0, 0);
  if (display_t > tm && display_t < 127 - bm) {
    ucg.drawLine(sm, display_t, 127 - fm, display_t);
  }
  int s = 0;
  if (threshold_found) {
    s = tf - 64;
    display_t = t;
    if (display_t > tm && display_t < 127 - bm) {
      ucg.setColor(0, 255, 255);
      ucg.drawLine(sm, display_t, 127 - fm, display_t);
    }
  }

  for (int x = sm; x < (127 - fm); x++) {
    ucg.setColor(0, 0, 0);
    for (int i = 0; i < cs; i++) {
      if (pa[i][x] > tm && pa[i][x] < 127 - bm && pa[i][x + 1] > tm && pa[i][x + 1] < 127 - bm) {
        ucg.drawLine(x, pa[i][x], x + 1, pa[i][x + 1]);
      }
    }
    for (int i = 0; i < cs; i++) {
      a[i][s + 1] = max(0,min(127, a[i][s + 1]));
      if (a[i][s] > tm && a[i][s] < 127 - bm && a[i][s + 1] > tm && a[i][s + 1] < 127 - bm) {
        ucg.setColor(color[i][0], color[i][1], color[i][2]);
        ucg.drawLine(x, a[i][s], x + 1, a[i][s + 1]);
      }
      pa[i][x] = a[i][s];
    }
    s++;
  }
  for (int i = 0; i < cs; i++) {
    pa[i][127 - fm] = a[i][s];
  }

  // set channels effected after garbage collection on display
  if (cs != set_channels) {
    cs = set_channels;
  }

  frames++;

  disp = false;
}

void setup_scale(bool looped, bool black) {

  float scale_division = max_d / (max_v / (float)sf);
  int divisions = ceil(max_d / scale_division);

  if(!looped){
    ucg.setColor(255, 255, 255);
  }else if(black){
    ucg.setColor(0, 0, 0);
  }else{
    ucg.setColor(0, 63, 63);
  }

  if(!looped){
    ucg.drawLine(0, 64, sm - 1, 64);
    ucg.drawLine(sm - 1, 0, sm - 1, 127);
  
    ucg.drawLine((127 - fm + 1), 64, 127, 64);
    ucg.drawLine((127 - fm + 1), 0, (127 - fm + 1), 127);
  
    ucg.drawLine(sm, tm, (127 - fm), tm);
    ucg.drawLine(sm, 127 - bm, (127 - fm), 127 - bm);
  }

  for (int i = 0; i < divisions; i++) {
    int d = round(i * scale_division);

    if(!looped){
      ucg.drawLine(0, 64 - d, sm - 1, 64 - d);
      ucg.drawLine(0, 64 + d, sm - 1, 64 + d);
  
      ucg.setPrintPos(0, 64 - d - 1);
      ucg.print(i);
      ucg.setPrintPos(0, 64 + d - 1);
      ucg.print(i);
  
      ucg.drawLine((127 - fm + 1), 64 - d, 127, 64 - d);
      ucg.drawLine((127 - fm + 1), 64 + d, 127, 64 + d);
    }else{
      if(64 - d > tm + 1){
        ucg.drawLine(sm, 64 - d, 127 - fm - 1, 64 - d);
      }
      if(64 + d < 127 - bm - 1){
        ucg.drawLine(sm, 64 + d, 127 - fm - 1, 64 + d);
      }
    }
  }

  // if sf > 1, add more subdivisions of volts

  float ts = 0.0;
  if(tb < 25){
      ts = 100.0;
  }else if(tb < 50){
      ts = 200.0;
  }else if(tb < 100){
      ts = 500.0;
  }else if(tb < 200){
      ts = 1000.0;
  }else if(tb <= 1000){
      ts = 5000.0;
  }
  
  float pixels_per_ts = ts / (float)tb;
  int t_divisions = ceil(max_d / pixels_per_ts);

  if(black){
    ucg.setColor(0, 0, 0);
  }else if(looped){
    ucg.setColor(63, 0, 63);
  }

  for (int i = 0; i < t_divisions; i++) {
    int d = round(i * pixels_per_ts);

    if(!looped){
      if(64 - d > sm){
        ucg.drawLine(64 - d, 127 - bm, 64 - d, 127);
        ucg.drawLine(64 - d, 0, 64 - d, tm);
      }
      if(64 + d < 127 - fm){
        ucg.drawLine(64 + d, 127 - bm, 64 + d, 127);
        ucg.drawLine(64 + d, 0, 64 + d, tm);
      }
    }else{
      if(64 - d > sm){
        ucg.drawLine(64 - d, tm + 1, 64 - d, 127 - bm - 1);
      }
      if(64 + d < 127 - fm){
        ucg.drawLine(64 + d, tm + 1, 64 + d, 127 - bm - 1);
      }
    }
  }

  if(looped){
    // show where offsets are 

    //(((offsets[i]) >> (6 - sf)) - sf_adj[sf]);
  }

  if(!looped){
    ucg.setPrintPos(66, 128);
    ucg.print((int)ts);
  }
}

void refresh_grid(){
  setup_scale(true, true);
}

