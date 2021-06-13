/*
 * To upload sketches to DAEMON,
 * Please see instructions in https://github.com/Blinken-Lights/ASCII/tree/main/Daemon/hardware
 */

#define F_CPU 8000000  // This is used by delay.h library

#include <avr/io.h>        // Adds useful constants
#include <util/delay.h>    // Adds delay_ms and delay_us functions
#include <EEPROM.h>

#define EXT_RESET (1 << EXTRF)

const static byte scales[7][60] PROGMEM = {{0,2,2,4,4,5,7,7,9,9,11,11,12,14,14,16,16,17,19,19,21,21,23,23,24,26,26,28,28,29,31,31,33,33,35,35,36,38,38,40,40,41,43,43,45,45,47,47,48,50,50,52,52,53,55,55,57,57,59,59},
        {0,2,2,3,5,5,7,7,8,10,10,12,12,14,14,15,17,17,19,19,20,22,22,24,24,26,26,27,29,29,31,31,32,34,34,36,36,38,38,39,41,41,43,43,44,46,46,48,48,50,50,51,53,53,55,55,56,58,58,60},
        {0,2,2,3,5,5,7,7,8,11,11,11,12,14,14,15,17,17,19,19,20,23,23,23,24,26,26,27,29,29,31,31,32,35,35,35,36,38,38,39,41,41,43,43,44,47,47,47,48,50,50,51,53,53,55,55,56,59,59,59},
        {0,2,2,3,5,5,7,7,9,9,10,12,12,14,14,15,17,17,19,19,21,21,22,24,24,26,26,27,29,29,31,31,33,33,34,36,36,38,38,39,41,41,43,43,45,45,46,48,48,50,50,51,53,53,55,55,57,57,58,60},
        {0,2,2,3,7,7,7,7,8,12,12,12,12,14,14,15,19,19,19,19,20,24,24,24,24,26,26,27,31,31,31,31,32,36,36,36,36,38,38,39,43,43,43,43,44,48,48,48,48,50,50,51,55,55,55,55,56,60,60,60},
        {0,2,2,4,4,7,7,7,9,9,12,12,12,14,14,16,16,19,19,19,21,21,24,24,24,26,26,28,28,31,31,31,33,33,36,36,36,38,38,40,40,43,43,43,45,45,48,48,48,50,50,52,52,55,55,55,57,57,60,60},
        {0,2,2,3,6,6,6,7,8,11,11,11,12,14,14,15,18,18,18,19,20,23,23,23,24,26,26,27,30,30,30,31,32,35,35,35,36,38,38,39,42,42,42,43,44,47,47,47,48,50,50,51,54,54,54,55,56,59,59,59},
        };

const byte hhat_sample[] PROGMEM = {123,125,124,123,123,124,125,123,124,125,125,123,122,125,123,122,121,123,124,121,123,124,125,123,123,125,126,123,125,126,128,125,125,128,126,125,121,126,120,121,119,120,123,119,125,123,130,124,129,130,129,129,124,129,121,123,117,121,119,117,121,119,128,120,131,128,132,131,125,133,120,125,116,120,119,116,122,119,130,122,135,130,138,135,130,138,120,130,111,120,110,110,113,109,124,112,135,124,145,135,140,146,127,141,111,127,106,110,108,105,118,107,129,118,144,129,148,145,139,149,120,140,108,119,105,107,113,103,121,112,132,121,138,132,140,138,132,140,123,132,116,123,115,115,116,115,122,116,126,122,130,126,127,130,125,127,124,125,131,124,136,131,134,136,123,134,114,123,109,114,112,109,116,111,129,116,143,129,155,144,150,156,134,151,115,134,106,114,107,105,117,106,127,117,129,127,118,129,109,118,111,109,124,110,139,124,154,140,163,155,162,165,143,163,119,144,101,118,102,100,112,100,124,111,127,124,123,127,114,123,114,114,121,113,131,120,135,131,136,135,136,136,139,136,136,140,122,136,102,122,96,101,109,95,131,108,145,132,152,146,153,154,148,155,130,149,114,130,111,114,119,110,119,119,109,118,103,108,113,102,129,112,148,129,168,149,186,170,178,188,140,180,88,140,58,86,60,55,83,57,113,81,147,112,177,148,188,179,170,191,135,172,101,135,81,100,78,79,93,76,123,91,158,122,177,159,168,179,139,171,108,139,94,108,100,92,117,99,132,117,131,132,118,132,104,117,108,103,128,107,152,127,168,153,171,169,155,173,121,157,81,121,61,79,73,58,112,71,156,111,189,158,193,192,167,197,121,169,83,121,64,81,67,62,85,64,122,84,165,121,190,166,178,193,142,181,109,142,97,108,99,95,111,98,134,111,159,134,165,161,141,167,105,142,85,104,90,83,108,89,126,107,142,126,151,143,146,152,129,147,117,129,119,116,126,118,124,126,117,124,117,117,123,117,124,123,119,124,121,118,136,121,148,136,144,149,131,145,119,131,109,119,96,108,92,95,108,90,141,107,169,142,177,171,164,180,138,165,109,139,85,108,77,83,93,74,129,91,175,129,216,178,243,220,249,249,229,254,181,233,119,183,62,119,28,59,22,24,41,18,72,38,101,69,116,100,120,116,120,120,126,120,135,126,150,136,164,151,167,165,153,169,133,154,120,133,120,120,119,120,115,119,116,115,126,115,133,126,130,134,124,130,129,124,137,129,136,137,131,137,136,131,144,136,144,145,133,144,130,133,136,130,136,136,127,137,126,127,140,125,151,140,138,153,111,139,95,111,101,94,116,100,133,116,155,133,185,156,202,187,190,206,145,193,92,146,54,91,46,51,65,43,106,62,154,105,192,155,200,195,175,203,133,178,93,133,70,92,76,68,107,73,148,106,172,149,173,174,161,176,149,162,136,150,123,136,116,122,121,115,128,121,124,128,111,124,105,111,114,104,134,113,152,134,163,153,158,164,136,159,106,137,86,105,86,84,110,84,150,109,198,151,227,201,221,232,177,226,120,179,76,120,58,73,64,55,98,62,143,96,178,144,176,180,145,178,109,146,93,108,95,91,113,94,140,112,169,140,183,171,174,186,147,176,120,147,102,120,94,101,99,93,119,98,137,118,138,137,125,139,121,125,133,121,154,134,166,155,173,168,172,175,156,174,122,158,94,122,88,92,106,86,128,106,147,128,158,148,161,160,142,162,112,142,90,112,93,88,115,91,146,114,170,147,183,172,173,185,149,175,121,150,103,120,96,102,98,94,105,97,120,104,133,120,141,133,140,142,139,140,141,140,147,142,145,148,136,146,122,137,112,121,107,112,115,106,132,114,155,132,166,157,157,167,130,159,110,130,102,109,107,101,118,107,138,118,155,139,157,156,139,159,120,140,112,120,117,111,123,117,133,123,141,133,144,141,135,145,124,135,121,124,130,120,140,130,147,140,146,148,139,147,119,140,97,119,89,96,106,87,139,105,167,139,173,169,157,175,128,159,101,128,83,100,87,81,112,86,148,111,175,149,184,177,170,187,138,172,97,139,69,96,70,67,100,68,135,98,161,135,169,163,162,171,142,164,124,143,119,124,128,118,136,128,132,136,118,132,109,117,108,108,116,107,131,116,153,131,167,154,162,168,134,163,102,134,84,101,87,82,106,85,140,106,171,141,183,173,165,186,131,166,101,131,87,99,88,86,108,86,141,107,170,141,169,173,140,171,105,140,93,104,101,91,119,100,140,118,165,140,175,167,157,177,118,158,94,118,96,92,114,95,127};
const byte kick_sample[] PROGMEM = {123,124,123,124,123,124,123,123,123,123,123,123,123,123,123,123,123,123,122,123,122,123,122,122,122,122,122,122,122,122,121,122,121,121,121,121,121,121,121,121,120,121,120,121,120,120,120,120,120,120,120,120,119,120,119,120,119,120,119,119,119,119,119,119,118,119,118,119,118,119,118,118,118,118,118,118,118,118,117,118,117,118,117,117,117,117,116,117,116,117,116,117,116,117,116,116,115,116,115,116,115,116,115,116,115,116,115,116,114,115,114,115,114,115,114,115,114,115,113,114,113,114,113,114,113,114,113,114,112,113,112,113,112,113,112,113,111,112,111,112,111,112,111,112,111,112,111,112,110,111,110,111,110,103,110,83,102,55,81,40,52,52,36,67,49,58,65,48,55,65,45,84,62,84,83,71,82,58,68,44,56,26,41,17,24,17,18,18,19,18,19,19,20,20,21,21,22,21,22,22,23,24,24,25,25,25,26,26,27,27,28,27,29,27,29,28,29,28,30,29,30,30,31,31,32,32,33,33,34,34,35,34,36,34,36,34,36,34,36,35,37,36,37,36,38,37,39,38,40,38,40,39,41,39,41,40,42,41,43,42,44,43,44,44,45,45,46,45,47,46,48,47,49,48,50,50,51,51,52,52,53,52,54,53,55,54,55,56,56,57,57,58,59,60,60,61,61,63,63,64,64,66,65,67,67,69,68,70,70,71,71,73,72,74,73,76,75,78,76,80,78,82,80,83,81,85,83,88,84,90,86,92,88,95,89,98,91,102,93,106,94,109,96,111,98,114,100,116,102,119,104,121,106,124,108,126,111,128,114,129,116,130,118,131,119,131,120,133,121,134,123,136,125,138,127,142,130,145,132,139,134,137,133,137,132,137,133,137,132,137,133,139,134,139,135,146,136,183,141,231,177,247,229,237,250,212,241,186,217,163,189,145,164,136,145,133,135,133,134,144,133,168,139,193,163,209,190,217,208,227,216,235,229,239,242,242,250,244,254,245,254,245,254,244,254,242,254,240,254,238,254,236,252,235,250,233,249,232,248,231,248,230,246,228,245,227,244,226,243,226,241,225,240,224,239,223,239,222,238,222,237,221,236,220,236,220,235,219,234,219,234,218,233,218,233,217,232,217,232,216,231,216,230,215,230,215,229,214,229,214,228,214,228,213,227,213,227,212,226,212,226,211,225,211,225,210,224,210,224,210,223,209,223,209,222,208,222,208,221,208,221,207,220,207,220,206,219,206,219,205,218,205,218,205,217,204,217,204,216,203,216,203,215,203,215,202,214,202,214,201,213,201,213,201,213,200,212,200,212,200,211,199,211,199,210,198,210,198,209,198,209,197,208,197,208,196,207,196,207,196,207,195,206,195,206,195,205,194,205,194,204,194,204,193,203,193,203,192,203,192,202,192,202,191,201,191,201,191,200,190,200,190,199,190,199,189,199,189,198,189,198,188,197,188,197,187,197,187,196,187,196,186,195,186,195,186,194,185,194,185,194,185,193,184,193,184,192,184,192,183,191,181,191,178,190,178,189,179,189,180,188,179,188,178,188,177,188,173,187,170,185,165,180,159,174,152,167,144,162,145,158,153,157,159,158,159,160,155,161,144,160,128,155,110,145,94,132,85,124,84,123,88,127,97,133,112,140,118,143,115,143,108,142,95,139,79,125,62,103,47,82,35,67,29,52,30,38,31,35,44,40,61,45,66,54,69,66,74,79,80,87,81,83,79,72,59,57,36,38,34,28,34,29,35,29,35,30,37,31,37,31,37,32,37,32,37,32,37,33,37,33,38,34,38,34,39,34,39,34,40,34,40,35,40,36,41,36,41,37,42,37,43,37,43,38,43,38,43,38,44,38,44,39,44,39,45,40,45,41,45,41,45,41,45,41,46,41,47,41,48,42,48,43,50,44,51,44,52,45,52,46,52,46,51,46,51,46,51,47,51,47,51,47,51,47,52,47,53,48,54,48,55,49,55,50,56,51,56,51,56,51,56,52,57,52,57,52,58,52,58,53,58,53,59,54,59,55,60,56,60,56,60,56,61,56,61,56,61,57,62,57,62,58,63,58,63,59,64,59,64,59,65,59,65,60,66,60,66,61,67,61,68,61,68,62,69,63,69,63,70,63,70,63,71,63,71,63,72,64,73,64,74,65,75,65,77,66,78,66,79,66,80,67,80,67,81,68,82,68,83,69,84,69,86,69,87,70,88,70,89,70,90,71,91,72,93,72,94,73,95,74,96,74,98,75,99,76,100,77,102,77,103,78,105,79,107,80,108,81,110,81,112,82,114,83,116,84,118,85,120,86,122,87};

int count = 1;
int max_apps = 10;

byte pwm1 = 0;
byte pwm2 = 0;

// setup timer interrupt if app needs it
unsigned long t1_f = 0;
//word wave_f = 0; // ISR frequency x 10 (to allow fractional frequencies without floating point). So max is 6553Hz
//word wave_low_f = 0; // ISR frequency x 1000 (to allow fractional frequencies without floating point). So max is 65Hz
byte bit_depth = 8; // max 8 bit
byte wave_inc = 1 << (8 - bit_depth);

// used for trig detection
bool cvj3_state = false;
bool prev_cvj3_state = false;

//variables for specific apps
int buf = 0;
bool pulse_state = true;
bool gate_state = false;
byte wave_height = 0;
byte prev_wave_height = 0;
byte pulse_ticks = 0;
int t = 0;
int q = 0;
unsigned long lt = 0;
byte euclidean_clock = 0;
byte euclidean[4]; // stores 32 bits in 4 bytes
byte prev_dv = 0;
byte prev_len = 0;
byte voct = 0;
byte prev_voct = 0;

void setup() {

  count = EEPROM.read(0);
  if(count > max_apps){
    count = 1;
  }

  // borrowed from https://sites.google.com/site/wayneholder/using-the-avr-reset-pin-as-an-input 'Wayne's Tinkering Page'
  if ((MCUSR & EXT_RESET) != 0) {
    // If External RESET then increment count
    MCUSR = MCUSR & ~EXT_RESET;
    count++;
    if(count > max_apps){
      count = 1;
    }
    EEPROM.write(0, count);
  }

  // setup different stuff depending on which app chosen by 'count'
  
  pinMode(0, OUTPUT); // OC0A
  pinMode(1, OUTPUT); // OC0B
  pinMode(2, INPUT); // Pot 1 - either input_pullup or analogue input
  pinMode(3, INPUT); // Pot 2
  pinMode(4, INPUT); // potential V/Oct

  // flash outputs to signify chosen app

  for(int i=0; i<count; i++){
    digitalWrite(0, HIGH);
    digitalWrite(1, HIGH);
    _delay_ms(125);
    digitalWrite(0, LOW);
    digitalWrite(1, LOW);
    _delay_ms(125);
  }
  _delay_ms(250);

  // The knobs normalled to 5V will glitch if another external cv input goes above/below 0-5V
  //      put it on different vreg / vref so it doesn't have a diode voltage added / removed from it
  // for audio-rate sampling
  ADCSRA = _BV(ADEN)    // enable the ADC
       | _BV(ADPS1)   // clock at
       | _BV(ADPS0);  //     F_CPU / 8 = 1 MHz
    
  // setup PWM if app needs it
  /*
  Control Register A for Timer/Counter-0 (Timer/Counter-0 is configured using two registers: A and B)
  TCCR0A is 8 bits: [COM0A1:COM0A0:COM0B1:COM0B0:unused:unused:WGM01:WGM00]
  2<<COM0A0: sets bits COM0A0 and COM0A1, which (in Fast PWM mode) clears OC0A on compare-match, and sets OC0A at BOTTOM
  2<<COM0B0: sets bits COM0B0 and COM0B1, which (in Fast PWM mode) clears OC0B on compare-match, and sets OC0B at BOTTOM
  3<<WGM00: sets bits WGM00 and WGM01, which (when combined with WGM02 from TCCR0B below) enables Fast PWM mode
  */
  TCCR0A = 2<<COM0A0 | 2<<COM0B0 | 3<<WGM00;
 
  /*
  Control Register B for Timer/Counter-0 (Timer/Counter-0 is configured using two registers: A and B)
  TCCR0B is 8 bits: [FOC0A:FOC0B:unused:unused:WGM02:CS02:CS01:CS00]
  0<<WGM02: bit WGM02 remains clear, which (when combined with WGM00 and WGM01 from TCCR0A above) enables Fast PWM mode
  1<<CS00: sets bits CS01 (leaving CS01 and CS02 clear), which tells Timer/Counter-0 to not use a prescalar
  */
  TCCR0B = 0<<WGM02 | 1<<CS00;

  OCR0A = 0;
  OCR0B = 0;

  randomSeed(analogRead(0));

  if(count == 1){
    vclfo_setup();
  }else if(count == 2){
    ad_env_setup();
  }else if(count == 3){
    ar_env_setup();
  }else if(count == 4){
    quantiser_setup();
  }else if(count == 5){
    turing_setup();
  }else if(count == 6){
    bytebeat_setup();
  }else if(count == 7){
    bytebeatg_setup();
  }else if(count == 8){
    bernoulli_setup();
  }else if(count == 9){
    drum_setup();
  }else if(count == 10){
    euclidean_setup();
  }

  // if timer 1 frequency is set, start timer 1 interrupt
  if(t1_f > 0){
    cli();
    TCCR1 = 0;
    // turn on CTC mode
    TCCR1 |= (1 << CTC1);
    
    if(t1_f == 8000000){
      TCCR1 |= (1 << CS10); // CK / 1
    }else if(t1_f == 4000000){
      TCCR1 |= (1 << CS11); // CK / 2
    }else if(t1_f == 2000000){
      TCCR1 |= (1 << CS10);
      TCCR1 |= (1 << CS11); // CK / 4
    }else if(t1_f == 1000000){
      TCCR1 |= (1 << CS12); // CK / 8
    }else if(t1_f == 500000){
      TCCR1 |= (1 << CS12);
      TCCR1 |= (1 << CS10); // CK / 16
    }else if(t1_f == 250000){
      // for 8-bit wave frequency between 3.84Hz and 980.39Hz
      // if lower bit depth is chosen, the available frequency range will start higher
      TCCR1 |= (1 << CS12);
      TCCR1 |= (1 << CS11); // CK / 32
    }else if(t1_f == 7813){ // really 8MHz/1024 = 7812.5, used for experimental bytebeat 8kHz
      TCCR1 |= (1 << CS13);
      TCCR1 |= (1 << CS11);
      TCCR1 |= (1 << CS10); // CK / 1024
    }
    
    TIMSK |= (1 << OCIE1A);

    /*if(wave_low_f > 0){
      set_low_f(wave_low_f);
    }else if(wave_f > 0){
      set_f(wave_f);
    }*/
    sei();
  }
}

ISR(TIM1_COMPA_vect){
  if(count == 1){
    vclfo_interrupt();
  }else if(count == 2){
    ad_env_interrupt();
  }else if(count == 3){
    ar_env_interrupt();
  }else if(count == 6){
    bytebeat_interrupt();
  }else if(count == 7){
    bytebeatg_interrupt();
  }else if(count == 9){
    drum_interrupt();
  }
}

void loop() {

  if(count == 1){
    vclfo_loop();
  }else if(count == 2){
    ad_env_loop();
  }else if(count == 3){
    ar_env_loop();
  }else if(count == 4){
    quantiser_loop();
  }else if(count == 5){
    turing_loop();
  }else if(count == 6){
    bytebeat_loop();
  }else if(count == 7){
    bytebeatg_loop();
  }else if(count == 8){
    bernoulli_loop();
  }else if(count == 9){
    drum_loop();
  }else if(count == 10){
    euclidean_loop();
  }

  trig_loop();
}


// app callbacks

// VCLFO
void vclfo_setup(){
  // start interrupt at 250kHz
  t1_f = 250000;
}
void vclfo_loop(){
  OCR1C = 255 - (pulse_state?(analogRead(1) >> 2):(analogRead(3) >> 2));
  
  if(rise_trig()){
    pulse_state = true;
    wave_height = 0;
    OCR0A = 255;
  }
}
void vclfo_interrupt(){
  wave_height+=(pulse_state)?1:-1;
  OCR0B = wave_height;
  if(wave_height >= 255){
    pulse_state = false;
    OCR0A = 0;
  }else if(wave_height <= 0){
    pulse_state = true;
    OCR0A = 255;
  }
}

// AD ENV
void ad_env_setup(){
  // start interrupt at 250kHz
  t1_f = 250000;
}
void ad_env_loop(){
  OCR1C = 255 - (pulse_state?(analogRead(1) >> 2):(analogRead(3) >> 2));
  
  if(rise_trig()){
    pulse_state = true;
  }
}
void ad_env_interrupt(){
  if(pulse_state || wave_height > 0){
    wave_height+=(pulse_state)?1:-1;
  }
  OCR0B = wave_height;
  if(wave_height >= 255){
    pulse_state = false;
  }else if(!pulse_state && prev_wave_height > 0 && wave_height==0){
    OCR0A = 255;
    pulse_ticks = 0;
  }
  if(pulse_ticks < 255){
    pulse_ticks++;
    OCR0A = 255 - pulse_ticks;
  }

  prev_wave_height = wave_height;
}

// AR ENV
void ar_env_setup(){
  // start interrupt at 250kHz
  t1_f = 250000;
}
void ar_env_loop(){
  gate_state = bitRead(PINB, 4);
  OCR1C = 255 - (gate_state?(analogRead(1) >> 2):(analogRead(3) >> 2));
}
void ar_env_interrupt(){
  if(gate_state && wave_height < 255){
    wave_height++;
  }else if(!gate_state && wave_height > 0){
    wave_height--;

    if(prev_wave_height > 0 && wave_height == 0){
      OCR0A = 255;
      pulse_ticks = 0;
    }

  }else if(!gate_state && pulse_ticks < 255){
    pulse_ticks++;
    OCR0A = 255 - pulse_ticks;
  }
  prev_wave_height = wave_height;
  OCR0B = wave_height;
}

// QUANTISER
void quantiser_setup(){
  
}
void quantiser_loop(){
  if(rise_trig()){
    byte scale = analogRead(3) >> 7; // 0 - 7
    byte note = quantise_note(analogRead(1));
    byte scale_note = to_scale(note, scale);
    prev_voct = voct;
    voct = to_analogue(scale_note);
    OCR0A = voct;
    OCR0B = prev_voct;
  }else if(fall_trig()){
    
  }
  prev_cvj3_state = cvj3_state;
}

// TURING
void turing_setup(){
  
}
void turing_loop(){
  if(rise_trig()){
    int chance_input = analogRead(1);
    byte scale = 0;
    byte l = analogRead(3) >> 6;
    bool dice_throw = random(1023) < chance_input;

    bool pulse = bitRead(buf, l);
    buf = buf << 1;
    if(dice_throw){
      pulse = random(1023) < 512;
    }
    
    //add the end of the buffer (or the new bit) to the start
    bitWrite(buf, 0, pulse);

    byte note = min(60, (buf & 15)); // select a 4 bit value (0-63) and clip to 0-60
    if(pulse){
      OCR0A = 255;
    }
    OCR0B = to_analogue(to_scale(note, scale));
  }else if(fall_trig()){
    OCR0A = 0;
  }
}

// BYTEBEAT FILTER
void bytebeat_setup(){
  // start interrupt at 250kHz
  t1_f = 250000;
}
void bytebeat_loop(){
  
}
void bytebeat_interrupt(){
  t = analogRead(1);
  q = analogRead(3);
  byte bd = analogRead(2) >> 7;
  OCR0B = (((t>>6>>t|7>>t^(t%(t/20)))|(6>>t>>5)) * q >> bd) << bd;
  //OCR0B = (((t*(t>>8&t) | t*(t>>q&t)&128) * q) >> bd) << bd;
  OCR0A = (OCR0B>127)?255:0;
}

// BYTEBEAT GENERATOR
void bytebeatg_setup(){
  // start interrupt at 250kHz
  t1_f = 250000;
}
void bytebeatg_loop(){
  OCR1C = 255 - (analogRead(1) >> 2); // speed
  q = analogRead(3) >> 6;
  if(rise_trig()){ // sync
    lt = 0;
  }
}
void bytebeatg_interrupt(){
  lt++;
  OCR0B = lt*(lt>>8&lt) | lt*(lt>>q&lt)&128;
  OCR0A = (OCR0B>127)?255:0;
}

// BERNOULLI
void bernoulli_setup(){
  
}
void bernoulli_loop(){
  if(rise_trig()){
    int chance_input = analogRead(1);
    bool dice_throw = random(1023) > chance_input;

    if(dice_throw){
      OCR0A = 255;
      OCR0B = 0;
    }else{
      OCR0A = 0;
      OCR0B = 255;
    }
  }else if(fall_trig() && (analogRead(3) >> 9)){
    OCR0A = 0;
    OCR0B = 0;
  }
}

// DRUM
// replace with drum sample
void drum_setup(){
  // start interrupt at 250kHz
  t1_f = 250000;
  OCR0B = 127;
}
void drum_loop(){
  OCR1C = 255 - (analogRead(1) >> 2); // speed
  q = analogRead(3) >> 8; // sample choice
  if(rise_trig()){
    t = 1023;
  }
}
void drum_interrupt(){
  if(t > 0){
    int pos = (q==0 || q==3)?(1023-t):t;
    OCR0B = (q>>1)?pgm_read_byte_near(hhat_sample + pos):pgm_read_byte_near(kick_sample + pos);
    t--;
  }else{
    OCR0B = 127;
  }
}

// EUCLIDEAN
void euclidean_setup(){
  
}
void euclidean_loop(){
  byte dv = analogRead(3) >> 6; // 0-15
  byte len = analogRead(3) >> 5; // 0-31

  if(prev_dv != dv || prev_len != len){
    fill_euclidean(len, dv);
  }
  prev_dv = dv;
  prev_len = len;
  
  if(rise_trig()){
    OCR0A = read_euclidean(euclidean_clock)?255:0;
    OCR0B = read_euclidean(len - euclidean_clock)?255:0;

    euclidean_clock++;
    if(euclidean_clock >= len){
      euclidean_clock = 0;
    }
  }else if(fall_trig()){
    OCR0A = 0;
    OCR0B = 0;
  }
}

// utility functions

bool rise_trig(){
  cvj3_state = bitRead(PINB, 4);
  return (cvj3_state && !prev_cvj3_state);
}
bool fall_trig(){ // always call rise_trig first, or this won't work
  return (!cvj3_state && prev_cvj3_state);
}
void trig_loop(){
  prev_cvj3_state = cvj3_state;
}

byte quantise_note(int value){
  return min(60,round((float)((value >> 2) + 2) / 4.25));
}

byte quantise(int value){
  return to_analogue(quantise_note(value));
}

byte to_analogue(byte note){
  return round(note * 4.25);
}

byte to_scale(byte note, byte scale){
  if(scale == 0){ // 0 = chromatic, which is a waste to store in memory
     return note;
  }else{
     scale--;
     return pgm_read_byte_near(&(scales[scale][note]));
  }
}

word to_f(byte note){ // returns 10x frequency to avoid returning a float
  return round( pow(2, ((float)note / 12.0f) ) * 550.0f );
}

word v_to_f(int adc){ // returns 10x frequency to avoid returning a float
  return round( pow(2, (adc * 0.00488758553f) ) * 550.0f );
}

/*void set_f(word f){
  if(t1_f <= 0 || f <= 0){
    return;
  }

  int compare_timer = round( t1_f / ((f/10) * (256/wave_inc)) );
  if(compare_timer > 255){
    compare_timer = 255;
  }

  OCR1C = compare_timer;
}
void set_low_f(word f){
  if(t1_f <= 0 || f <= 0){
    return;
  }

  int compare_timer = round( t1_f / ((f/1000) * (256/wave_inc)) );
  if(compare_timer > 255){
    compare_timer = 255;
  }

  OCR1C = compare_timer;
}*/

bool read_euclidean(byte stp){
  byte byt = stp >> 3;         // stp divided by 8
  byte bt = stp % 8;
  
  return bitRead(euclidean[byt], bt);
}
void write_euclidean(byte stp, bool on_off){
  byte byt = stp >> 3;         // stp divided by 8
  byte bt = stp % 8;
  
  bitWrite(euclidean[byt], bt, on_off);
}

void fill_euclidean(byte len, byte pls){
  byte bucket = 0;
  for(int i=0; i < len; i++) {
    bucket+= pls;

    if (bucket >= len) {
      bucket-= len;
      //euclidean_array[len-i-1] = 1;
      write_euclidean(len-i-1, 1);
    }else if(bucket < len){
      //euclidean_array[len-i-1] = 0;
      write_euclidean(len-i-1, 0);
    }
  }
}
