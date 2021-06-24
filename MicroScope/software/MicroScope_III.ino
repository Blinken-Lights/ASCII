//MicroScope III

/* tested working on chinese STM3

   Board Library:   Arduino_STM32
          github:   https://github.com/rogerclarkmelbourne/Arduino_STM32
   Board:           Generic STM32F103C series
   Optimise:        Fastest (-O3)
   Variant:         20k RAM, 64k flash
   CPU Speed:       72MHz
   Upload method:   STLink
   
   Compile, then upload with:
   st-flash --debug --reset write [compiled binary location] 0x8000000

  jumpers both set to 0
*/

/*
 * TO DO:
 Y * Setting for AC/DC (shift plus SW3)
 Y * Setting for number of channels (shift plus SW2, page through 1, 2, or 4)
 Y * Coloured bar indicators above each jack - show / hide depending on number of channels
 Y * Setting to stack channels (shift plus SW3)
 Y *  - stacked channels in non-scan mode
 Y *  - scale for stacked channels
 Y * Setting for lissajous (shift plus SW4)
 Y *  - scale for lissajous (voltage along bottom too)
 Y * limit zoom level to best quality
 * 
 Y * Swap up/down buttons
 Y * Hardcode timebase to useful intervals (ie double each increment?)
 Y *  - Change sample rate on different intervals - longer the better (increase myADC.setSampleRate(ADC_SMPR_7_5));
 Y * When timebase is very slow, display a partially sampled array on screen [bool scan]
 Y *    (ie go to 'display' routine before sample array is overwritten, and only sample 128 datapoints)
 *    
 Y * better scale label for slow timebases
 *
 *
 Y * debug calibration
 Y *    - problem is with timebase 15
 * 
 Y * set defaults back to fast stacked with no lissajous
 */


#include <EEPROM.h>
#include <SPI.h>
#include "Ucglib.h"
#include <STM32ADC.h>

STM32ADC myADC(ADC1);
uint16_t dataPoints[4];

Ucglib_SSD1351_18x128x128_HWSPI ucg(/*cd=*/ PB0, /*cs=*/ PA4, /*reset=*/ PB1);

// in microseconds
uint32 timebases[16] = { 
                          234375, // 30 s per whole screen
                          117187, // 15 s
                           58593, //  7 s
                           29296, //  3 s
                           
                           14648, //  1.5 s
                            7324, //  0.75 s
                            3662, //  0.375 s
                            1831, //  0.1875 s
                            
                             781, //  0.1 s
                             390, //  0.05 s
                             195, //  0.025 s
                              98, //  0.0125 s
                              
                              49, //  0.00625 s
                              24, //  0.003125 s
                              20, //  0.00256 s
                              10  //  0.00128 s
                        };
// scan mode
bool scanmode[16] = { 
                              true, // 30 s per whole screen
                              true, // 15 s
                              true, //  7 s
                              true, //  3 s
                              true, //  1.5 s
                              true, //  0.75 s
                              true, //  0.375 s
                              false, //  0.1875 s
                              false, //  0.1 s
                              false, //  0.05 s
                              false, //  0.025 s
                              false, //  0.0125 s
                              false, //  0.00625 s
                              false, //  0.003125 s
                              false, //  0.00256 s
                              false  //  0.00128 s
                        };
// sample rates .... could also change prescaler to make it more accurate for slow framerates
adc_smp_rate sample_rates[16] = {
                  ADC_SMPR_239_5, // 30 s per whole screen
                  ADC_SMPR_239_5, // 15 s
                  ADC_SMPR_239_5, //  7 s
                  ADC_SMPR_239_5, //  3 s
                  ADC_SMPR_239_5, //  1.5 s
                  ADC_SMPR_239_5, //  0.75 s
                   ADC_SMPR_71_5, //  0.375 s
                   ADC_SMPR_55_5, //  0.1875 s
                   ADC_SMPR_41_5, //  0.1 s
                   ADC_SMPR_28_5, //  0.05 s
                   ADC_SMPR_28_5, //  0.025 s
                   ADC_SMPR_13_5, //  0.0125 s
                   ADC_SMPR_13_5, //  0.00625 s
                    ADC_SMPR_7_5, //  0.003125 s
                    ADC_SMPR_7_5, //  0.00256 s
                    ADC_SMPR_7_5  //  0.00128 s
                        };

char *time_units[] = {
                      "us",
                      "ms",
                      "s "
                      };
uint32 time_divisors[] = {
                      1,
                      1000,
                      1000000
                      };

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
//bool remove_dc[] = { true, true, true, true };
bool remove_dc[] = { false, false, false, false };
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
float max_v = 5.2;
int sf = 1; // scale factor, minimum 1!
int sf_adj[7] = { 0, 0, 64, 192, 448, 960, 1984 };

//int tb = 9; // 9 microseconds minimum
//int tb = 10;
int tb = 10;
int tb_choice = 11;

//buttons
int debounce = 200;
int buttons[] = { PB14, PB15, PB12, PB13, PA8 };
unsigned long last_read[] = { 0, 0, 0, 0, 0 };
int presses[] = { 0, 0, 0, 0, 0 };
int dpresses[] = { 0, 0, 0, 0, 0 };
unsigned long held[] = { 0, 0, 0, 0, 0 };
bool down[] = { false, false, false, false };

byte mode = 0; // set with shift
// (like o/c, press shift within .. seconds of powering on module, to calibrate 0)
bool calibration_available = true;
byte calibrating_channel = 0;
int calibration_window = 5000; // milliseconds from powering on module

int sample = 0;
bool disp = false;
unsigned long last_sample = 0; // for scan mode

bool stack = true;
bool lissajous = false;
byte rightshift[5] = { 0, 0, 1, 1, 2 };

//debugging
int a0 = 0;

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
  myADC.setSampleRate(ADC_SMPR_7_5);//sample_rates[tb_choice]);//set the Sample Rate (try setting dynamically for different timebases) - max usable frequency at ADC_SMPR_7_5 is up to 27kHz and much less crosstalk than 1.5 cycles
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

  load_calibration();

  setup_timebase();

  ucg.setColor(255, 255, 255);
  ucg.setPrintPos(sm + 2, 127 - bm - 2);
  ucg.print("SHIFT TO CALIBRATE");
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
  bpress(0); // left
}
void button1(){
  bpress(1); // right
}
void button2(){
  bpress(2); // up
}
void button3(){
  bpress(3); // down
}
void button4(){
  bpress(4); // shift
}
void bpress(int i){
  unsigned long n = millis();
  if(n - last_read[i] < debounce){
    return;
  }
  last_read[i] = n;
  held[i] = n;
  down[i] = true;

  presses[i]++;
}

void loop(void)
{

  // can do some things outside of waveform display

  if(calibration_available){
    if(millis() > calibration_window){
      calibration_available = false;
      ucg.setColor(0,0,0);
      ucg.setPrintPos(sm + 2, 127 - bm - 2);
      ucg.print("SHIFT TO CALIBRATE");
    }
  }

  for(int i=0; i<5; i++){
    unsigned long n = millis();
    bool state = digitalRead(buttons[i]);
    if(!state){
      int held_for = n - held[i];
      if(held_for > debounce){
        presses[i]++;
        held[i] = n;
      }
    }else{
      down[i] = false;
    }
    
    if(dpresses[i] != presses[i]){

      if(i==0 || i==1){ // left / right
        if(mode == 2){ // zero calibration
          
        }else if(down[4]){ // if shift
          if(i==0){ // toggle AC/DC
            for(int z=0; z<4; z++){
              remove_dc[z] = !remove_dc[z];
            }
          }else if(i==1){
            lissajous = !lissajous;
            if(lissajous){
              stack = false;
            }
            if(lissajous && set_channels==1){
              set_channels = 2;
            }
            ucg.clearScreen(); // clear old scale
            setup_scale(true, false); // draw new scale
            setup_scale(false, false);
          }
        }else{ // increase / decrease horizontal axis scale ( timebase )
          tb_choice+= ((i==0)?1:-1);
          tb_choice = min(15,max(0,tb_choice));
          setup_timebase();
        }
      }else if(i==2 || i==3){ // up / down
        if(mode == 2){ // zero calibration
          
        }else if(down[4]){ // if shift
          if(i==2){ // number of channels
            set_channels++;
            if(lissajous && set_channels==1){
              set_channels = 2;
            }else if(set_channels==3){ // numbers available = 1, 2 or 4
              set_channels = 4;
            }else if(set_channels>=5){
              set_channels = 1;
            }
          }else if(i==3){
            stack = !stack;
            if(stack){
              lissajous = false;
            }
            ucg.clearScreen(); // clear old scale
            setup_scale(true, false); // draw new scale
            setup_scale(false, false);
          }
        }else{ // increase / decrease vertical axis scale ( voltage )
          sf+= ((i==2)?-1:1);
          sf = min(3,max(1,sf));
          ucg.clearScreen(); // clear old scale
          setup_scale(true, false); // draw new scale
          setup_scale(false, false);
        }
      }else if(i==4){ // shift
        if(mode!=2){
          if(calibration_available){
            calibrating_channel = 0;
            start_calibration();
          }
        }
      }
      
      dpresses[i] = presses[i];
    }
  }

  if(!disp && !scanmode[tb_choice]){
    return;
  }

  int debug = a[0][64];

  if(scanmode[tb_choice]){
    unsigned long microtime = micros();
    if(microtime - last_sample > timebases[tb_choice]){
      last_sample = microtime;
      
      for (int i = 0; i < cs; i++) {
        a[i][sample] = dataPoints[i];
      }

      /*if(frames > refresh_bg){
        setup_scale(true, false);
        frames = 0;
      }*/

      if(sample > sm && sample < (128-fm)){
        for (int i = 0; i < cs; i++) {
          a[i][sample]-=zero_offsets[i];
          a[i][sample] = ((((a[i][sample] + offsets[i]) + 2048) >> (6 - sf)) - sf_adj[sf]);

          //apply lp filter after rescaling, so it should look ok at any resolution
          a[i][sample]-=64;
          lp[i] = (sample>0)?( (lp_filter * a[i][sample]) + ((1.0-lp_filter)*lp[i]) ):a[i][sample];
          a[i][sample] = lp[i] + 64;

          // scale for stacking
          if(stack){
            a[i][sample] = (a[i][sample] >> rightshift[cs]) + ((128 / (cs*2)) * (i*2));
          }
        }

        if(lissajous){
          ucg.setColor(0, 0, 0);
          for (int i = 0; i < cs; i+=2) {
            if ((pa[i][sample - 1] > tm && pa[i][sample - 1] < 127 - bm && pa[i][sample] > tm && pa[i][sample] < 127 - bm)
                 && (pa[i+1][sample - 1] > tm && pa[i+1][sample - 1] < 127 - bm && pa[i+1][sample] > tm && pa[i+1][sample] < 127 - bm)){
              ucg.drawLine(pa[i][sample-1], pa[i+1][sample-1], pa[i][sample], pa[i+1][sample]);
            }
          }
          for (int i = 0; i < cs; i+=2) {
            a[i][sample] = max(0,min(127, a[i][sample]));
            a[i+1][sample] = max(0,min(127, a[i+1][sample]));
            if ((a[i][sample-1] > tm && a[i][sample-1] < 127 - bm && a[i][sample] > tm && a[i][sample] < 127 - bm)
                && (a[i+1][sample-1] > tm && a[i+1][sample-1] < 127 - bm && a[i+1][sample] > tm && a[i+1][sample] < 127 - bm)) {
              int color_index = (sample%2)?(i+1):i;
              ucg.setColor(color[color_index][0], color[color_index][1], color[color_index][2]);
              ucg.drawLine(a[i][sample - 1], a[i+1][sample-1], a[i][sample], a[i+1][sample]);
            }
            pa[i][sample - 1] = a[i][sample - 1];
            pa[i+1][sample - 1] = a[i+1][sample - 1];
          }
          if(sample == (127-fm)){
            for (int i = 0; i < cs; i++) {
              pa[i][sample] = a[i][sample];
              pa[i+1][sample] = a[i+1][sample];
            }
          }
        }else{
          ucg.setColor(0, 0, 0);
          for (int i = 0; i < cs; i++) {
            if (pa[i][sample - 1] > tm && pa[i][sample - 1] < 127 - bm && pa[i][sample] > tm && pa[i][sample] < 127 - bm) {
              ucg.drawLine(sample-1, pa[i][sample-1], sample, pa[i][sample]);
            }
          }
          for (int i = 0; i < cs; i++) {
            a[i][sample] = max(0,min(127, a[i][sample]));
            if (a[i][sample-1] > tm && a[i][sample-1] < 127 - bm && a[i][sample] > tm && a[i][sample] < 127 - bm) {
              ucg.setColor(color[i][0], color[i][1], color[i][2]);
              ucg.drawLine(sample - 1, a[i][sample-1], sample, a[i][sample]);
            }
            pa[i][sample - 1] = a[i][sample - 1];
          }
          if(sample == (127-fm)){
            for (int i = 0; i < cs; i++) {
              pa[i][sample] = a[i][sample];
            }
          }
        }
      }
      
      sample++;
      if(sample >= 128){
        sample = 0;
      }
    }
  }

  if(mode == 2){ // calibrating for 0
    float filter_sensitivity = 0.1;
    float avg_value = a[calibrating_channel][0];
    for (int x = 0; x < 511; x++) {
      if(a[calibrating_channel][x] != 0){
        avg_value = (filter_sensitivity * a[calibrating_channel][x]) + ((1.0-filter_sensitivity)*avg_value);
      }
    }
    zero_offsets[calibrating_channel] = round(avg_value);

    ucg.setColor(255,255,255);
    ucg.setPrintPos(sm + 2, 127 - bm - 2);
    char buff[50];
    sprintf(buff, "AVG: %d", round(avg_value));
    ucg.print(buff);
    delay(1000);
    
    calibrating_channel++;
    if(calibrating_channel>=4){
      stop_calibration();
    }
  }

  if(!scanmode[tb_choice]){
    non_scan_draw_lines();
  }

  // set channels effected after garbage collection on display
  if (cs != set_channels) {
    cs = set_channels;
    ucg.clearScreen();
    setup_scale(true, false); // draw new scale
    setup_scale(false, false);
  }


  /*
  // debugging to fine tune voltage measurement
  float error = (float)zero_offsets[0]-2048.0;
  float debug_reading = (4096.0 - (float)debug) + error - 2048.0;
  float voltage = (max_v / 2048.0) * debug_reading;
  ucg.setColor(255,255,255);
  ucg.setPrintPos(20,80);
  ucg.print(error);
  ucg.setPrintPos(20,100);
  ucg.print(voltage);
  ucg.setPrintPos(50,100);
  ucg.print(debug);
  ucg.setPrintPos(100,100);
  ucg.print(debug_reading);*/

  frames++;

  disp = false;
}

void setup_scale(bool looped, bool black) {

  float scale_division = max_d / (max_v / (float)(1 << (sf-1)));
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

  for (int i = 0; i < divisions; i+=(stack?cs:1)) {
    int d = round(i * scale_division);
    if(stack){
      d = round(d / cs);
    }

    if(!looped){

      for(int j = 0; j < (stack?cs:1); j++){
        int mp = stack?(((128 / (cs*2)) * ((j*2) + 1))):64;
        ucg.drawLine(0, mp - d, sm - 1, mp - d);
        ucg.drawLine(0, mp + d, sm - 1, mp + d);
    
        ucg.setPrintPos(0, mp - d - 1);
        ucg.print(i);
        ucg.setPrintPos(0, mp + d - 1);
        ucg.print(i);
    
        ucg.drawLine((127 - fm + 1), mp - d, 127, mp - d);
        ucg.drawLine((127 - fm + 1), mp + d, 127, mp + d);
      }

      if(lissajous){
        /*ucg.drawLine(64 - d, 126, 64 - d, 128 - bm);
        ucg.drawLine(64 + d, 126, 64 + d, 128 - bm);*/
    
        if(i>0){
          ucg.setPrintPos(64 - d + 2, 126);
          ucg.print(i);
        }
        ucg.setPrintPos(64 + d - 5, 126);
        ucg.print(i);
    
        ucg.drawLine(64 - d, (127 - bm), 64 - d, 126);
        ucg.drawLine(64 + d, (127 - bm), 64 + d, 126);
      }
    }else{
      if(64 - d > tm + 1){
        ucg.drawLine(sm, 64 - d, 127 - fm - 1, 64 - d);
      }
      if(64 + d < 127 - bm - 1){
        ucg.drawLine(sm, 64 + d, 127 - fm - 1, 64 + d);
      }

      if(lissajous){
        if(64 - d > sm + 1){
          ucg.drawLine(64 - d, 127, 64 - d, 127 - bm - 1);
        }
        if(64 + d < 127 - fm - 1){
          ucg.drawLine(64 + d, 127, 64 + d, 127 - bm - 1);
        }
      }
    }
  }

  // if sf > 1, add more subdivisions of volts

  int t_divisions = 8;
  float pixels_per_ts = 128 / t_divisions; // = 16.0

  if(!lissajous){
    /*if(tb < 25){
        ts = 100.0;
    }else if(tb < 50){
        ts = 200.0;
    }else if(tb < 100){
        ts = 500.0;
    }else if(tb < 200){
        ts = 1000.0;
    }else if(tb < 1000){
        ts = 5000.0;
    }else{
        ts = tb * 5.0;
    }
    
    float pixels_per_ts = ts / (float)tb;
    int t_divisions = ceil(max_d / pixels_per_ts);*/

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
  }else{
    if(looped){
      ucg.setColor(63, 0, 63);
      if(black){
        ucg.setColor(0, 0, 0);
      }
  
      for (int i = 0; i < divisions; i++) {
        int d = round(i * scale_division);
        if(64 - d > sm){
          ucg.drawLine(64 - d, tm + 1, 64 - d, 127 - bm - 1);
        }
        if(64 + d < 127-fm){
          ucg.drawLine(64 + d, tm + 1, 64 + d, 127 - bm - 1);
        }
      }
    }
  }

  if(looped){
    // show where offsets are 

    //(((offsets[i]) >> (6 - sf)) - sf_adj[sf]);
  }

  if(!looped && !lissajous){
    byte displayed_units = 0;
    uint32 ts = pixels_per_ts * timebases[tb_choice];
    for(int z=0; z<3; z++){
      if(ts > time_divisors[z]){
        displayed_units = z;
      }
    }
    float timescale = (float)ts / time_divisors[displayed_units];
    char ts_string[20];
    sprintf(ts_string, "%.2f %s", timescale, time_units[displayed_units]);
    ucg.setPrintPos(66, 126);
    ucg.print(ts_string);
  }
  if(!looped){
    refresh_channel_bars();
  }
}

void refresh_grid(){
  setup_scale(true, true);
}

void start_calibration(){
  mode = 2;
  for(int i=0; i<4; i++){
    remove_dc[i] = false; // turn off 'remove dc' because its impossible to zero calibrate with it on
  }

  tb_choice = 15; // to sample fast
  setup_timebase(); // and clears screen..
  
  print_calibrating_message();
}
void print_calibrating_message(){
  ucg.setColor(255,255,255);
  ucg.setPrintPos(sm + 2, 127 - bm - 2);
  ucg.print("DO NOT PLUG INTO JACKS");
}
void stop_calibration(){
  // save calibration to EEPROM
  for(int i=0; i<4; i++){
    EEPROM.write(i, zero_offsets[i]);
  }
  EEPROM.write(4, 127); // 'flag' to tell if module has ever been calibrated

  /*for(int i=0; i<4; i++){
    remove_dc[i] = true; // turn on 'remove dc' because it's on by default when the scope is powered on
  }*/

  mode = 0;

  ucg.setColor(255,255,255);
  ucg.setPrintPos(sm + 2, 127 - bm - 2);
  ucg.print("CALIBRATION COMPLETE");
  delay(2000);
  
  // redraw scale
  ucg.clearScreen(); // clear old scale
  setup_scale(true, false); // draw new scale
  setup_scale(false, false);
}
void load_calibration(){
  if(EEPROM.read(4) != 127){ // if first calibration has not been written
    for(int i=0; i<4; i++){
      zero_offsets[i] = 2048;
    }
  }else{
    for(int i=0; i<4; i++){
      zero_offsets[i] = EEPROM.read(i);
    }
  }
}

void setup_timebase(){
  tb = timebases[tb_choice];
  //myADC.setSampleRate(sample_rates[tb_choice]);
  //myADC.startConversion();
  ucg.clearScreen(); // clear old scale
  sample = 0;
  // if scanmode, turn off interrupt
  if(scanmode[tb_choice]){
    Timer2.pause();
    disp = true;
  }else{
    Timer2.resume();
    disp = false;
  }
  Timer2.setPeriod(tb);
  setup_scale(true, false); // draw new scale
  setup_scale(false, false);
}

void non_scan_draw_lines(){
  bool threshold_found = false;
  
  // move all maths out of the sampling loop
  for (int x = 0; x < (scanmode[tb_choice]?128:512); x++) {
    for (int i = 0; i < cs; i++) {

      a[i][x]-=zero_offsets[i];

      dc[i] = (dc_filter * a[i][x]) + ((1.0-dc_filter)*dc[i]);
      
      a[i][x] = ((((a[i][x] + offsets[i] - (remove_dc[i]?(int)dc[i]:0)) + 2048) >> (6 - sf)) - sf_adj[sf]);

      //apply lp filter after rescaling, so it should look ok at any resolution
      a[i][x]-=64;
      lp[i] = (x>0)?( (lp_filter * a[i][x]) + ((1.0-lp_filter)*lp[i]) ):a[i][x];
      a[i][x] = lp[i] + 64;

      if (i == 0 && x > 64 && x < 448 // 448 = 512 - 64
          && !threshold_found && a[i][x] <= t
          && a[i][x - 1] > t) {
        tf = x;
        threshold_found = true;
      }

      // scale for stacking
      if(stack){
        a[i][x] = (a[i][x] >> rightshift[cs]) + ((128 / (cs*2)) * (i*2));
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
  if (threshold_found && !lissajous) {
    s = tf - 64;
    display_t = t;
    if (display_t > tm && display_t < 127 - bm) {
      ucg.setColor(0, 255, 255);
      ucg.drawLine(sm, display_t, 127 - fm, display_t);
    }
  }

  if(lissajous){
    for (int x = 0; x < 127; x++) {
      ucg.setColor(0, 0, 0);
      for (int i = 0; i < cs; i+=2) {
        if ((pa[i][x] > tm && pa[i][x] < 127 - bm && pa[i][x + 1] > tm && pa[i][x + 1] < 127 - bm)
            && (pa[i+1][x] > tm && pa[i+1][x] < 127 - bm && pa[i+1][x + 1] > tm && pa[i+1][x + 1] < 127 - bm)){
          ucg.drawLine(pa[i][x], pa[i+1][x], pa[i][x+1], pa[i+1][x+1]);
        }
      }
      for (int i = 0; i < cs; i+=2) {
        a[i][x + 1] = max(0,min(127, a[i][x + 1]));
        a[i+1][x + 1] = max(0,min(127, a[i+1][x + 1]));
        if ((a[i][s] > tm && a[i][s] < 127 - bm && a[i][s + 1] > tm && a[i][s + 1] < 127 - bm)
            && (a[i+1][s] > tm && a[i+1][s] < 127 - bm && a[i+1][s + 1] > tm && a[i+1][s + 1] < 127 - bm)){
          int color_index = (x%2)?(i+1):i;
          ucg.setColor(color[color_index][0], color[color_index][1], color[color_index][2]);
          ucg.drawLine(a[i][x], a[i+1][x], a[i][x+1], a[i+1][x+1]);
        }
        pa[i][x] = a[i][x];
        pa[i+1][x] = a[i+1][x];
      }
      s++;
    }
    for (int i = 0; i < cs; i+=2) {
      pa[i][127] = a[i][127];
      pa[i+1][127] = a[i+1][127];
    }
  }else{
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
  }
}

void refresh_channel_bars(){
  ucg.setColor(0, 0, 0);
  ucg.drawLine(0, 127, 127, 127);
  for (int i = 0; i < cs; i++) {
    ucg.setColor(color[i][0], color[i][1], color[i][2]);
    ucg.drawLine(i * 32, 127, (i+1)*32, 127);
  }
}

