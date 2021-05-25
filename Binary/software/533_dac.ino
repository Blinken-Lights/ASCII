#include  <SPI.h>

const byte scale_pin = A6;
int scale_analogue = 0;
const byte tolerance = 15;

const byte cs_dac = 10;

const byte o0 = 3;
const byte o1 = 4;
const byte o2 = 5;
const byte n0 = 6;
const byte n1 = 7;
const byte n2 = 8;

const byte trig1 = 9; //trigger on change

const byte o02 = A0;
const byte o12 = A1;
const byte o22 = A2;
const byte n02 = A3;
const byte n12 = A4;
const byte n22 = A5;

const byte trig2 = 12; //trigger on change

const static word scales[6][38] PROGMEM = {{0,128,256,320,448,576,704,768,896,1024,1088,1216,1344,1472,1536,1664,1792,1856,1984,2111,2239,2303,2431,2559,2623,2751,2879,3007,3071,3199,3327,3391,3519,3647,3775,3839,3967,4095},
        {0,128,192,320,448,512,640,768,896,960,1088,1216,1280,1408,1536,1664,1728,1856,1984,2048,2175,2303,2431,2495,2623,2751,2815,2943,3071,3199,3263,3391,3519,3583,3711,3839,3967,4031},
        {0,128,192,320,448,512,704,768,896,960,1088,1216,1280,1472,1536,1664,1728,1856,1984,2048,2239,2303,2431,2495,2623,2751,2815,3007,3071,3199,3263,3391,3519,3583,3775,3839,3967,4031},
        {0,128,192,320,448,576,640,768,896,960,1088,1216,1344,1408,1536,1664,1728,1856,1984,2111,2175,2303,2431,2495,2623,2751,2879,2943,3071,3199,3263,3391,3519,3647,3711,3839,3967,4031},
        {0,128,192,448,512,768,896,768,896,960,1216,1280,1536,1664,1536,1664,1728,1984,2048,2303,2431,2303,2431,2495,2751,2815,3071,3199,3071,3199,3263,3519,3583,3839,3967,3839,3967,4031},
        {0,128,256,448,576,768,896,768,896,1024,1216,1344,1536,1664,1536,1664,1792,1984,2111,2303,2431,2303,2431,2559,2751,2879,3071,3199,3071,3199,3327,3519,3647,3839,3967,3839,3967,4095},
        };
// add 2 more scales?

byte scale = 0;

byte abs_note_1 = 0;
byte abs_note_2 = 0;
byte prev_abs_note_1 = 0;
byte prev_abs_note_2 = 0;
unsigned long last_trig1 = 0;
unsigned long last_trig2 = 0;
bool trig1_on = false;
bool trig2_on = false;
byte trig_length = 15;

//testing
const byte test_button = 2;

void setup() {

  Serial.begin(250000);
  
  opt_mode(cs_dac, OUTPUT);
  
  SPI.begin();  
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  opt_mode(test_button, INPUT_PULLUP);
  opt_mode(o0, INPUT_PULLUP);
  opt_mode(o1, INPUT_PULLUP);
  opt_mode(o2, INPUT_PULLUP);
  opt_mode(n0, INPUT_PULLUP);
  opt_mode(n1, INPUT_PULLUP);
  opt_mode(n2, INPUT_PULLUP);

  opt_mode(o02, INPUT_PULLUP);
  opt_mode(o12, INPUT_PULLUP);
  opt_mode(o22, INPUT_PULLUP);
  opt_mode(n02, INPUT_PULLUP);
  opt_mode(n12, INPUT_PULLUP);
  opt_mode(n22, INPUT_PULLUP);

  opt_mode(trig1, OUTPUT);
  opt_mode(trig2, OUTPUT);

  WriteRegister(0, 0);
  WriteRegister(0, 1);

  /*for(int u=0; u<999; u++){
    for(int j=0; j<38; j++){
      WriteRegister(pgm_read_word(&(scales[scale][j])));
      delay(100);
    }
  }*/
}

void loop() {

  if(!opt_read(test_button)){
    WriteRegister(4095, 0);
    WriteRegister(4095, 1);

    //to trigger refresh when test button is released
    prev_abs_note_1 = 255;
    prev_abs_note_2 = 255;
  }else{

    byte octave = !opt_read(o0) | (!opt_read(o1) << 1) | (!opt_read(o2) << 2);
    byte note = !opt_read(n0) | (!opt_read(n1) << 1) | (!opt_read(n2) << 2);
    abs_note_1 = pgm_read_word(&(scales[scale][(octave * 7) + note]));

    if(abs_note_1 != prev_abs_note_1){
      WriteRegister(abs_note_1, 0);
      opt_write(trig1, HIGH);
      trig1_on = true;
      last_trig1 = millis();
    }
    prev_abs_note_1 = abs_note_1;

    byte octave2 = !opt_read(o02) | (!opt_read(o12) << 1) | (!opt_read(o22) << 2);
    byte note2 = !opt_read(n02) | (!opt_read(n12) << 1) | (!opt_read(n22) << 2);
    abs_note_2 = pgm_read_word(&(scales[scale][(octave2 * 7) + note2]));
    
    if(abs_note_2 != prev_abs_note_2){
      WriteRegister(abs_note_2, 1);
      opt_write(trig2, HIGH);
      trig2_on = true;
      last_trig2 = millis();
    }
    prev_abs_note_2 = abs_note_2;
  }

  if(trig1_on){
    if(millis() - last_trig1 > trig_length){
      opt_write(trig1, LOW);
      trig1_on = false;
    }
  }
  if(trig2_on){
    if(millis() - last_trig2 > trig_length){
      opt_write(trig2, LOW);
      trig2_on = false;
    }
  }

  int analogue = analogRead(scale_pin);
  if(abs(scale_analogue - analogue) > tolerance){
    scale_analogue = analogue;
    scale = scale_analogue >> 7;
    scale = min(5, scale);
  }
}

//------------DAC通信------------------------
void WriteRegister(unsigned int val, byte dac_channel){
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | (dac_channel << 7) | B00010000;

  bitClear(PORTB, cs_dac - 8);
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  bitSet(PORTB, cs_dac - 8);
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
