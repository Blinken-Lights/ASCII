#include <EEPROM.h>

// dependency:
// https://github.com/olikraus/u8g2
#include <U8g2lib.h>
#include <SPI.h>
U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);

#define ascii_logo_width 126
#define ascii_logo_height 41
static const unsigned char ascii_logo_bits[] U8X8_PROGMEM = {
   0x00, 0x7e, 0x00, 0x00, 0xfc, 0x01, 0x00, 0xf8, 0x03, 0x00, 0xe0, 0x07,
   0x00, 0xc0, 0x0f, 0x00, 0x00, 0x86, 0x00, 0x00, 0x0c, 0x01, 0x00, 0x18,
   0x02, 0x00, 0x30, 0x0c, 0x00, 0xe0, 0x18, 0x00, 0x00, 0x87, 0x00, 0x00,
   0x0e, 0x01, 0x00, 0x38, 0x06, 0x00, 0x70, 0x08, 0x00, 0xe0, 0x10, 0x00,
   0x00, 0x0f, 0x01, 0x00, 0x1e, 0x02, 0x00, 0x3c, 0x04, 0x00, 0x78, 0x18,
   0x00, 0xf0, 0x31, 0x00, 0x80, 0x0f, 0x01, 0x00, 0x1f, 0x02, 0x00, 0x7c,
   0x0c, 0x00, 0xf8, 0x10, 0x00, 0xf0, 0x21, 0x00, 0x80, 0x1f, 0x02, 0x00,
   0x3f, 0x04, 0x00, 0x7e, 0x08, 0x00, 0xf8, 0x30, 0x00, 0xf0, 0x63, 0x00,
   0xc0, 0x1f, 0x02, 0x80, 0x3f, 0x04, 0x00, 0xfe, 0x18, 0x00, 0xf8, 0x21,
   0x00, 0xe0, 0x43, 0x00, 0xc0, 0x3f, 0x04, 0x80, 0x7f, 0x08, 0x00, 0xff,
   0x10, 0x00, 0xf0, 0x61, 0x00, 0xe0, 0xc7, 0x00, 0xe0, 0x3f, 0x04, 0x80,
   0x7f, 0x08, 0x00, 0xff, 0x31, 0x00, 0xf0, 0x43, 0x00, 0xc0, 0x87, 0x00,
   0xe0, 0x7f, 0x08, 0xc0, 0xff, 0x10, 0x80, 0xff, 0x21, 0x00, 0xe0, 0xc7,
   0x00, 0xc0, 0x8f, 0x01, 0xf0, 0x7f, 0x08, 0xc0, 0xff, 0x10, 0x80, 0xff,
   0x63, 0x00, 0xe0, 0x87, 0x00, 0x80, 0x0f, 0x01, 0xf0, 0xfd, 0x10, 0xe0,
   0xf3, 0x21, 0xc0, 0xef, 0x43, 0x00, 0xc0, 0x87, 0x01, 0x80, 0x1f, 0x03,
   0xf8, 0xff, 0x10, 0xe0, 0xff, 0x21, 0xc0, 0xe7, 0xc7, 0x00, 0xc0, 0x0f,
   0x01, 0x00, 0x1f, 0x02, 0xf8, 0xf9, 0x21, 0xe0, 0xe3, 0x43, 0xe0, 0xc3,
   0x87, 0x00, 0xc0, 0x0f, 0x03, 0x00, 0x3f, 0x06, 0xf8, 0xf1, 0x21, 0xe0,
   0xe3, 0x43, 0xe0, 0xc3, 0x8f, 0x01, 0xc0, 0x1f, 0x02, 0x80, 0x3f, 0x04,
   0xfc, 0xf3, 0x43, 0xc0, 0xc7, 0x87, 0xf0, 0xe1, 0x0f, 0x01, 0xe0, 0x3f,
   0x06, 0x80, 0x7f, 0x0c, 0xfc, 0xe3, 0x43, 0xf8, 0xc7, 0x87, 0xf0, 0xa1,
   0x1f, 0xe3, 0xe3, 0x3f, 0xc4, 0xc7, 0x7f, 0x08, 0xfe, 0xc7, 0x87, 0x8c,
   0x8f, 0x0f, 0xf9, 0x30, 0x1f, 0x32, 0xf2, 0x3f, 0xec, 0xcc, 0xff, 0x18,
   0xfe, 0xc7, 0x87, 0x8c, 0x8f, 0x1f, 0xf9, 0x10, 0x3f, 0x76, 0xf6, 0x7f,
   0xe8, 0xe8, 0xff, 0x10, 0xdf, 0x8f, 0x0f, 0x1f, 0x1f, 0x1f, 0x7e, 0x18,
   0x3e, 0x7c, 0xfc, 0xfd, 0xf8, 0xf9, 0xfb, 0x31, 0x9f, 0x8f, 0xff, 0x1f,
   0x1f, 0xff, 0xff, 0x0f, 0xfe, 0xff, 0xfc, 0xf8, 0xff, 0xf1, 0xf3, 0x3f,
   0x8f, 0x9f, 0x0f, 0x3f, 0x3e, 0x1e, 0x7e, 0x08, 0x7c, 0xfc, 0xfc, 0xf8,
   0xf8, 0xfb, 0xf1, 0x21, 0x0f, 0xdf, 0x0f, 0x3f, 0x3e, 0x1e, 0x7e, 0x08,
   0x3c, 0xfc, 0x7d, 0x70, 0xe8, 0xfb, 0xe1, 0x31, 0x86, 0xff, 0x87, 0x7e,
   0x7c, 0x0c, 0xf9, 0x10, 0x38, 0xf6, 0x7f, 0x7c, 0xec, 0xff, 0xf0, 0x10,
   0xfe, 0xfe, 0x87, 0x7c, 0x7c, 0xfc, 0xf9, 0x10, 0xf8, 0xf3, 0x3f, 0xe4,
   0xc7, 0xff, 0xd8, 0x1f, 0x00, 0xfe, 0x43, 0xfc, 0xf8, 0x10, 0xf0, 0x21,
   0x00, 0xe0, 0x3f, 0x06, 0xc0, 0x7f, 0x08, 0x00, 0x00, 0xfc, 0x43, 0xf8,
   0xf8, 0x10, 0xf0, 0x61, 0x00, 0xe0, 0x1f, 0x02, 0x80, 0x7f, 0x0c, 0x00,
   0x00, 0xfc, 0x21, 0xf0, 0xf1, 0x21, 0xe0, 0x43, 0x00, 0xc0, 0x1f, 0x03,
   0x80, 0x3f, 0x04, 0x00, 0x00, 0xf8, 0x21, 0xf0, 0xf1, 0x3f, 0xe0, 0xc3,
   0x00, 0xc0, 0xff, 0x01, 0x00, 0xff, 0x07, 0x00, 0x00, 0xf8, 0x10, 0xe0,
   0xf3, 0x21, 0xc0, 0x87, 0x00, 0x80, 0x0f, 0x01, 0x00, 0x3f, 0x06, 0x00,
   0x00, 0xfc, 0x10, 0xe0, 0xf3, 0x21, 0xc0, 0x8f, 0x01, 0x80, 0x1f, 0x03,
   0x00, 0x3e, 0x04, 0x00, 0x00, 0x7c, 0x08, 0xc0, 0xff, 0x10, 0x80, 0x0f,
   0x01, 0x00, 0x1f, 0x02, 0x00, 0x7e, 0x0c, 0x00, 0x00, 0x7c, 0x08, 0xc0,
   0xff, 0x10, 0x80, 0x1f, 0x03, 0x00, 0x3f, 0x06, 0x00, 0x7c, 0x08, 0x00,
   0x00, 0x3e, 0x04, 0x80, 0x7f, 0x08, 0x00, 0x1f, 0x02, 0x00, 0x3e, 0x04,
   0x00, 0xfc, 0x18, 0x00, 0x00, 0x3e, 0x04, 0x80, 0x7f, 0x08, 0x00, 0x3f,
   0x06, 0x00, 0x7e, 0x0c, 0x00, 0xf8, 0x10, 0x00, 0x00, 0x1f, 0x02, 0x00,
   0x3f, 0x04, 0x00, 0x3e, 0x04, 0x00, 0x7c, 0x08, 0x00, 0xf8, 0x31, 0x00,
   0x00, 0x1f, 0x02, 0x00, 0x3f, 0x04, 0x00, 0xfe, 0x0f, 0x00, 0xfc, 0x1f,
   0x00, 0xf0, 0x3f, 0x00, 0x80, 0x0f, 0x01, 0x00, 0x1e, 0x02, 0x00, 0x7c,
   0x0c, 0x00, 0xf8, 0x18, 0x00, 0xf0, 0x21, 0x00, 0x00, 0x0f, 0x01, 0x00,
   0x1e, 0x02, 0x00, 0x3c, 0x04, 0x00, 0x78, 0x08, 0x00, 0xe0, 0x31, 0x00,
   0x00, 0x87, 0x00, 0x00, 0x0c, 0x01, 0x00, 0x38, 0x06, 0x00, 0x70, 0x0c,
   0x00, 0xe0, 0x10, 0x00, 0x00, 0xfe, 0x00, 0x00, 0xfc, 0x01, 0x00, 0xf8,
   0x03, 0x00, 0xf0, 0x07, 0x00, 0xc0, 0x1f, 0x00 };

volatile byte adc_raw[2];
byte data[2][256];
int data_counter;
unsigned char input_index=0;
bool sampling = false;
float smoothing = 0.5;
float f = 0.0f;
float f_smoothing = 0.4;

byte timebaseplus_sw = 5;
byte timebaseminus_sw = 4;
byte mode_sw = 3;
byte show_hz_sw = 2;

byte timebase_addr = 0;
byte mode_addr = 1;
byte show_hz_addr = 2;

int timebase = 0; // 0 - 18
bool incremental_display = false;
int strt = 1;
int periods = 0;
int total_period_samples = 0;
int last_rise = 0;
byte max_timebase = 19;
const static word timebase_ocrs[20] PROGMEM = {
        199,
        399,
        799,
       1599,
       3199,
       6399,
      12799,
      25599,
      51199,
      12799,
      25599,
      51199,
      12799,
      25599,
      51199,
      25599,
      51199,
      25599,
      51199
    };
const static word timebase_prescalers[20] PROGMEM = {
          1,
          1,
          1,
          1,
          1,
          1,
          1,
          1,
          1,
          8,
          8,
          8,
         64,
         64,
         64,
        256,
        256,
       1024,
       1024
    };
const static float timebase_frequencies[20] PROGMEM = {
      80000.0,
      40000.0,
      20000.0,
      10000.0,
       5000.0,
       2500.0,
       1250.0,
        625.0,
        312.5,
        156.25,
         78.125,
         39.0625,
         19.53125,
          9.765625,
          4.8828125,
          2.44140625,
          1.220703125,
          0.6103515625,
          0.30517578125
    };
unsigned long timebase_changed = 0;
unsigned long mode_changed = 0;
byte mode = 0; // 0 = rising edge triggered, 1 = vectorscope
bool show_hz = false;

bool prev_timebaseplus;
bool prev_timebaseminus;
bool prev_mode;
bool prev_show_hz;

void setup() {

  /*  The bit of code below was taken from:
   *  http://yaab-arduino.blogspot.it/p/oscope.html
   */
  ADCSRA = 0;             // clear ADCSRA register
  ADCSRB = 0;             // clear ADCSRB register
  ADMUX |= (6 & 0x07);    // set A6 analog input pin
  ADMUX |= (1 << REFS0);  // set reference voltage
  ADMUX |= (1 << ADLAR);  // left align ADC value to 8 bits from ADCH register

  // sampling rate is [ADC clock] / [prescaler] / [conversion clock cycles]
  // for Arduino Uno ADC clock is 16 MHz and a conversion takes 13 clock cycles
  //ADCSRA |= (1 << ADPS2) | (1 << ADPS0);    // 32 prescaler for 38.5 KHz
  ADCSRA |= (1 << ADPS2);                     // 16 prescaler for 76.9 KHz
  //ADCSRA |= (1 << ADPS1) | (1 << ADPS0);    // 8 prescaler for 153.8 KHz
  //ADCSRA |= (1 << ADPS0);    // 2 prescaler

  //ADCSRA |= (1 << ADATE); // enable auto trigger // turned off, for single conversion mode
  ADCSRA |= (1 << ADIE);  // enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN);  // enable ADC
  ADCSRA |= (1 << ADSC);  // start ADC measurements

  for (uint8_t i = 0; i<8; i++){
    DIDR0 |= 1<<i;
  }
  
  // button inputs
  pinMode(timebaseplus_sw, INPUT_PULLUP);
  pinMode(timebaseminus_sw, INPUT_PULLUP);
  pinMode(mode_sw, INPUT_PULLUP);
  pinMode(show_hz_sw, INPUT_PULLUP);
  
  pinMode(9, OUTPUT);
  digitalWrite(9, 0);
  u8g2.begin();
  u8g2.setFlipMode(1);
  u8g2.setFont(u8g_font_unifont);

  timebase = EEPROM.read(timebase_addr);
  int maxtimebaseminus1 = max_timebase - 1;
  timebase = min(maxtimebaseminus1, timebase);
  timebase = max(0, timebase);
  mode = EEPROM.read(mode_addr);
  mode = min(1, mode);
  mode = max(0, mode);
  show_hz = EEPROM.read(show_hz_addr) == 1;

  // show ASCII logo for 3 seconds
  u8g2.firstPage();
  do {
    u8g2.drawXBMP( 0, 11, ascii_logo_width, ascii_logo_height, ascii_logo_bits);
  } while ( u8g2.nextPage() );
  delay(3000);

  set_timebase();
}

//
// **************************************************************************
// *
// *      A D C _ I S R
// *
// **************************************************************************
//
// ADC interrupt service routine
// with auto input scanning
// after Lee (theusch) https://www.avrfreaks.net/forum/how-read-multiple-adc-channels-atmega328
ISR(ADC_vect){

  //if(!sampling){
  //  return;
  //}

// Read the AD conversion result
   adc_raw[input_index]=ADCH;
   
// Select next ADC input
   if (++input_index >= 2){
      input_index=0;
   }

   ADMUX= B01100000 | +(input_index + 6);

// Start the AD conversion
   ADCSRA |= 0x40;

}

ISR(TIMER1_COMPA_vect){

  if(!sampling){
    return;
  }
  
  data[0][data_counter] = adc_raw[0];
  data[1][data_counter] = adc_raw[1];

  if(incremental_display && data_counter >=1){
    if(data_counter == 1){
      reset_signal_processing_vars();
    }
    process_signal_at_x(data_counter);
  }

  data_counter++;
  if(data_counter == 256 && !incremental_display){
    sampling = false;
    data_counter = 0;
  }else if(incremental_display && data_counter > 128){
    data_counter = 0;
    calculate_frequency();
  }
}

void set_timebase(){
  int prescaler = pgm_read_word(&(timebase_prescalers[timebase]));
  
  // old MicroScope ran at 40000 Hz
  // this version runs at max 80000 Hz
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 80000 Hz increments
  OCR1A = pgm_read_word(&(timebase_ocrs[timebase])); // = 16000000 / (1 * 80000) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  if(prescaler == 1){
    // Set CS12, CS11 and CS10 bits for 1 prescaler
    TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
  }else if(prescaler == 8){
    // Set CS12, CS11 and CS10 bits for 8 prescaler
    TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
  }else if(prescaler == 64){
    // Set CS12, CS11 and CS10 bits for 64 prescaler
    TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10);
  }else if(prescaler == 256){
    // Set CS12, CS11 and CS10 bits for 64 prescaler
    TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10);
  }else if(prescaler == 1024){
    // Set CS12, CS11 and CS10 bits for 1024 prescaler
    TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);
  }
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts

  // It may be possible to increase the analogue reading quality by changing the ADC prescaler in ADCSRA
  // for different time bases

  incremental_display = timebase >= 8;

  EEPROM.update(timebase_addr, timebase);

  timebase_changed = millis();
}

void check_switches(){
  bool timebaseplus_sw_state = !opt_read(timebaseplus_sw);
  bool timebaseminus_sw_state = !opt_read(timebaseminus_sw);
  bool mode_sw_state = !opt_read(mode_sw);
  bool show_hz_sw_state = !opt_read(show_hz_sw);

  if(timebaseplus_sw_state && !prev_timebaseplus){
    timebase++;
    if(timebase >= max_timebase){
      timebase = 0;
    }
    set_timebase();
  }else if(timebaseminus_sw_state && !prev_timebaseminus){
    timebase--;
    if(timebase < 0){
      timebase = max_timebase-1;
    }
    set_timebase();
  }else if(mode_sw_state && !prev_mode){
    mode++;
    if(mode > 1){
      mode = 0;
    }
    EEPROM.update(mode_addr, mode);
    mode_changed = millis();
  }else if(show_hz_sw_state && !prev_show_hz){
    show_hz = !show_hz;
    EEPROM.update(show_hz_addr, show_hz?1:0);
  }

  prev_timebaseplus = timebaseplus_sw_state;
  prev_timebaseminus = timebaseminus_sw_state;
  prev_mode = mode_sw_state;
  prev_show_hz = show_hz_sw_state;
}

void reset_signal_processing_vars(){
  strt = 1;
  periods = 0;
  total_period_samples = 0; // total to divide by periods, to find average period
  last_rise = 0;
}

void process_signal_at_x(int x){
  for(byte ch=0; ch<2; ch++){
    float y1 = (float)data[ch][x-1];
    float y2 = (float)data[ch][x];

    data[ch][x] = max(0,min(255, round((smoothing * y2) + ((1.0f-smoothing) * y1)))); //use min/max to constrain result to a byte

    // base edge triggering and frequency calculation on channel 0 / Channel A
    if(ch > 0){
      continue;
    }
    
    //find zero crossing for display
    if(x > 63 && strt == 1 && data[ch][x] <= 127 && data[ch][x-1] > 127){
      strt = x - 63; // start display half a screen before zero crossing, IE place zero crossing in the centre of the screen
    }

    //find zero crossing for frequency
    if(data[ch][x] <= 127 && data[ch][x-1] > 127){
      if(last_rise == 0){
        last_rise = x;
      }else{
        int last_period = x - last_rise;
        total_period_samples+= last_period;
        periods++;
        last_rise = x;
      }
    }
  }
}

void calculate_frequency(){
  f_smoothing = incremental_display?0.95:0.4;
  
  float this_f = 0.0;
  float sample_frequency = pgm_read_float(&(timebase_frequencies[timebase])); // take sample_frequency from array based on timebase
  if(periods > 0){
    float avg_period = (float)total_period_samples / (float)periods; // measured in 'samples'
    float avg_period_seconds = avg_period * (1.0f / sample_frequency);
    this_f = 1.0f / avg_period_seconds;
  }
  f = (f_smoothing * this_f) + ((1.0f-f_smoothing) * f);
}

void loop() {

  check_switches();

  if(sampling && !incremental_display){
    return;
  }

  // smooth results and find zero crossing, if not in scan mode
  if(!incremental_display){
    reset_signal_processing_vars();
    for(int x=1; x<256; x++){
       process_signal_at_x(x);
    }
    calculate_frequency();
  }

  float sample_frequency = pgm_read_float(&(timebase_frequencies[timebase])); // take sample_frequency from array based on timebase
  float screenwidth = (1.0f/sample_frequency) * 128.0f;
  
  u8g2.firstPage();
  do {
    // draw lines
    byte data_x = incremental_display?3:strt;
    for(int x = 1; x<126; x++){
      if(mode == 0){
        for(byte ch = 0; ch<2; ch++){
          byte x1 = x-1;
          byte x2 = x;
          byte y1 = data[ch][data_x-1] >> 2;
          byte y2 = data[ch][data_x] >> 2;
  
          if((y1 == 0 && y2 == 0) || (y1 >= 63 && y2 >= 63)){
            continue;
          }
  
          u8g2.drawLine(x1, y1, x2, y2);
        }
      }else if(mode == 1){ // vectorscope
        byte x1 = data[0][data_x-1] >> 1; // scale the x plot x2 to cover the whole 128 pixel wide screen
        byte x2 = data[0][data_x] >> 1; //   scale the x plot x2 to cover the whole 128 pixel wide screen
        byte y1 = data[1][data_x-1] >> 2;
        byte y2 = data[1][data_x] >> 2;

        // bias the y plot towards the centre of the screen
        //.... or expand to 128 wide?
        //y1+= 32;
        //y2+= 32;

        u8g2.drawLine(x1, y1, x2, y2);
      }
      data_x++;
    }
  
    // draw text to screen if needed
    enum {BufSize=9};
    char buf[BufSize];
  
    // draw frequency
    if(show_hz){
      dtostrf(f, 9, 2, buf);
      u8g2.drawStr(0,64,buf);
      u8g2.drawStr(74,64,"Hz");
    }
  
    if(millis() - timebase_changed <= 3000){
      dtostrf(screenwidth, 9, 5, buf);
      u8g2.drawStr(0,10,buf);

      if(mode == 0){
        if(incremental_display){
          u8g2.drawStr(0,20,"Scan");
        }else{
          u8g2.drawStr(0,20,"Edge Triggered");
        }
      }

      /*if(incremental_display){ // just for debugging
        itoa(timebase, buf, 10);
        u8g2.drawStr(95,10,buf);
        u8g2.drawStr(64,10,"inc");
      }*/
    }else if(millis() - mode_changed <= 3000){
      if(mode == 0){
        u8g2.drawStr(0,10,"Time/Voltage Plot");
      }else if(mode == 1){
        u8g2.drawStr(0,10,"Vectorscope");
      }
    }
  } while ( u8g2.nextPage() );

  sampling = true;
}


bool opt_read(byte pin){
  if(pin >= 14){
    pin-=14;
    return bitRead(PINC, pin);
  }else if(pin >= 8){
    pin-=8;
    return bitRead(PINB, pin);
  }else{
    return bitRead(PIND, pin);
  }
}

void opt_write(byte pin, bool val){
  if(pin >= 14){
    pin-=14;
    bitWrite(PORTC, pin, val);
  }else if(pin >= 8){
    pin-=8;
    bitWrite(PORTB, pin, val);
  }else{
    bitWrite(PORTD, pin, val);
  }
}

void opt_mode(byte pin, byte val){
  if(val == 2){
    opt_write(pin, HIGH);
    val = 0;
  }
  if(pin >= 14){
    pin-=14;
    bitWrite(DDRC, pin, val);
  }else if(pin >= 8){
    pin-=8;
    bitWrite(DDRB, pin, val);
  }else{
    bitWrite(DDRD, pin, val);
  }
}
