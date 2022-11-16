/*
 * 8 Bit Waves Redesign 2022
 * with LFO mode
 * V1.0
 */

#include <EEPROM.h>
#include <MIDI.h>
#include <MozziGuts.h>
#include <mozzi_fixmath.h>
#include <IntMap.h>
#include <mozzi_midi.h>
#include <Oscil.h>
#include <ADSR.h>
#include <LowPassFilter.h>
#include <tables/square_no_alias_2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/sin2048_int8.h>

MIDI_CREATE_DEFAULT_INSTANCE();

#define debug false

#define CONTROL_RATE 100 //128 seems to cause glitches

Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> aSqu(SQUARE_NO_ALIAS_2048_DATA);
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> aSqu2(SQUARE_NO_ALIAS_2048_DATA);

Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> aSaw(SAW2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> aSaw2(SAW2048_DATA);

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin2(SIN2048_DATA);

ADSR <CONTROL_RATE, CONTROL_RATE> envelope;
ADSR <CONTROL_RATE, CONTROL_RATE> envelope2;

LowPassFilter lpf;

const IntMap oneEighty(0,255,0,180);
const IntMap twoHundred(0,255,0,200);
const IntMap twoHundredFive(0,255,0,205);

const static byte quantised_notes[1035] PROGMEM = {0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,53,53,53,53,53,53,53,53,53,53,53,53,53,53,53,53,53,54,54,54,54,54,54,54,54,54,54,54,54,54,54,54,54,54,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60};
const static word adc_centres[129] PROGMEM = {0,17,34,51,68,85,102,119,137,154,171,188,205,222,239,256,273,290,307,324,341,358,375,393,410,427,444,461,478,495,512,529,546,563,580,597,614,631,649,666,683,700,717,734,751,768,785,802,819,836,853,870,887,905,922,939,956,973,990,1007,1024,1041,1058,1075,1092,1109,1126,1143,1161,1178,1195,1212,1229,1246,1263,1280,1297,1314,1331,1348,1365,1382,1399,1417,1434,1451,1468,1485,1502,1519,1536,1553,1570,1587,1604,1621,1638,1655,1673,1690,1707,1724,1741,1758,1775,1792,1809,1826,1843,1860,1877,1894,1911,1929,1946,1963,1980,1997,2014,2031,2048,2065,2082,2099,2116,2133,2150,2167,2185};

const static float detune_float[127] PROGMEM = {0.5003,0.5122,0.5241,0.536,0.5479,0.5598,0.5717,0.5836,0.5955,0.6074,0.6193,0.6312,0.6431,0.655,0.6669,0.6788,0.6907,0.7026,0.7145,0.7264,0.7383,0.7502,0.7621,0.774,0.7859,0.7978,0.8097,0.8216,0.8335,0.8454,0.8573,0.8692,0.8811,0.893,0.9049,0.9168,0.9287,0.9406,0.9525,0.9644,0.9763,0.9882,1.0001,1.012,1.0239,1.0358,1.0477,1.0596,1.0715,1.0834,1.0953,1.1072,1.1191,1.131,1.1429,1.1548,1.1667,1.1786,1.1905,1.2024,1.2143,1.2262,1.2381,1.25,1.2619,1.2738,1.2857,1.2976,1.3095,1.3214,1.3333,1.3452,1.3571,1.369,1.3809,1.3928,1.4047,1.4166,1.4285,1.4404,1.4523,1.4642,1.4761,1.488,1.4999,1.5118,1.5237,1.5356,1.5475,1.5594,1.5713,1.5832,1.5951,1.607,1.6189,1.6308,1.6427,1.6546,1.6665,1.6784,1.6903,1.7022,1.7141,1.726,1.7379,1.7498,1.7617,1.7736,1.7855,1.7974,1.8093,1.8212,1.8331,1.845,1.8569,1.8688,1.8807,1.8926,1.9045,1.9164,1.9283,1.9402,1.9521,1.964,1.9759,1.9878,1.9997};

byte voct_pin     = A7;

byte arp_pin      = 1;

byte clk_pin      = 2;
byte wave1sw0     = 5;
byte wave1sw1     = 4;
byte wave2sw0     = 7;
byte wave2sw1     = 6;
byte mixsw0       = 8;
byte mixsw1       = 10;
byte clk_button   = 12;
byte learn_sw     = 13;

byte status_red   = 11;
byte status_blue  = 3;

byte quantised_note = 0;
byte midi_note = 0;
byte detune = 0;
byte env_detune = 0;
byte env_octave = 0;
byte prev_env_octave = 0;
byte env = 0;
byte env2 = 0;
int tolerance = 10;
int prev_analogue = 0;
int prev_analogue2 = 0;
int prev_analogue3 = 0;
int prev_analogue4 = 0;
bool clk_state_prev = false;
bool clk_button_state_prev = false;

// 0 = squ, 1 = saw, 2 = tri
byte wave1 = 0;
byte wave2 = 0;
// 0 = add, 1 = ringmod, 2 = multiply
byte mix = 0;
int8_t out = 0;
bool wave_display = true;
byte prev_wave1 = 0;
byte prev_wave2 = 0;
byte prev_mix = 0;

byte gain;
byte filt_env;

bool do_arp = false;
byte cutoff = 180;
byte res    = 200;

byte midi_input_note = 0;
byte midi_note_changed = false;
byte midi_channel = 0;
int midi_channel_addr = 0;
bool learning = false;
bool prev_learning = false;

bool lfo_mode = false;
bool lfo_mode_changed = false;
int lfo_mode_addr = 127;
bool changing_mode = false;
int changing_mode_timeout = 5000;
unsigned long started_both_pressed = 0;
bool flash_state = false;
unsigned long flash_changed = 0;
int flash_for = 500;
bool prev_do_arp = false;

unsigned long last_clock = 0;
float tempo_frequency = 0.0f;

void setup() {
  #if debug
    Serial.begin(250000);
  #endif
  
  pinMode(voct_pin, INPUT);

  pinMode(clk_pin, INPUT);

  pinMode(clk_button, INPUT_PULLUP);
  pinMode(wave1sw0, INPUT_PULLUP);
  pinMode(wave1sw1, INPUT_PULLUP);
  pinMode(wave2sw0, INPUT_PULLUP);
  pinMode(wave2sw1, INPUT_PULLUP);
  pinMode(mixsw0, INPUT_PULLUP);
  pinMode(mixsw1, INPUT_PULLUP);
  pinMode(learn_sw, INPUT); // external pulldown resistor needed

  pinMode(status_red, OUTPUT);
  pinMode(status_blue, OUTPUT);
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  OCR2A = 0;
  OCR2B = 0;

  startMozzi(CONTROL_RATE); // :)
  aSaw.setFreq(mtof(midi_note));
  aSaw2.setFreq(mtof(midi_note));
  aSin.setFreq(mtof(midi_note));
  aSin2.setFreq(mtof(midi_note));
  aSqu.setFreq(mtof(midi_note));
  aSqu2.setFreq(mtof(midi_note));
  
  envelope.setLevels(255,0,0,0);
  envelope2.setLevels(45,0,0,0);

  midi_channel = EEPROM.read(midi_channel_addr);
  #if debug
    Serial.print("retrieved midi channel: ");
    Serial.println(midi_channel);
  #endif

  lfo_mode = EEPROM.read(lfo_mode_addr) == 1;
  #if debug
    Serial.print("retrieved lfo_mode: ");
    Serial.println(lfo_mode?"on":"off");
  #endif
  
  // Connect the HandleNoteOn function to the library, so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(HandleNoteOn);  // Put only the name of the function
  MIDI.setHandleNoteOff(HandleNoteOff);  // Put only the name of the function
  // Initiate MIDI communications, listen to all channels (not needed with Teensy usbMIDI)
  #if !debug
    MIDI.begin(midi_channel);
  #endif
  MIDI.turnThruOff();

  pinMode(arp_pin, INPUT_PULLUP);
}

void updateControl(){
  // put changing controls in here

  int analogue = mozziAnalogRead(A0);
  int analogue2 = mozziAnalogRead(A1) + mozziAnalogRead(A4);
  int analogue3 = mozziAnalogRead(A2) + mozziAnalogRead(A5);
  int analogue4 = mozziAnalogRead(A3) + mozziAnalogRead(A6);
  int voct = mozziAnalogRead(voct_pin);

  learning = opt_read(learn_sw);
  
  bool w1sw1 = !opt_read(wave1sw0);
  bool w1sw2 = !opt_read(wave1sw1);
  wave1 = (w1sw1 << 1) | (w1sw1?w1sw2:!w1sw2);
  
  bool w2sw1 = !opt_read(wave2sw0);
  bool w2sw2 = !opt_read(wave2sw1);
  wave2 = (w2sw1 << 1) | (w2sw1?w2sw2:!w2sw2);
  
  bool msw1 = !opt_read(mixsw0);
  bool msw2 = !opt_read(mixsw1);
  mix = (msw1 << 1) | (msw1?msw2:!msw2);

  #if debug
    if(prev_mix != mix){
      Serial.print("mix changed to: ");
      Serial.println(mix);
    }
    if(prev_wave1 != wave1){
      Serial.print("wave1 changed to: ");
      Serial.println(wave1);
    }
    if(prev_wave2 != wave2){
      Serial.print("wave2 changed to: ");
      Serial.println(wave2);
    }
  #endif

  do_arp = opt_read(arp_pin);

  wave_display = true;
  
  if(changing_mode){
    if(flash_changed == 0){
      flash_changed = millis();
    }else if((millis() - flash_changed) >= flash_for){
      flash_state = !flash_state;
      flash_changed = 0;
    }
    wave_display = false;
    if(lfo_mode){
      OCR2B = flash_state?255:0;
      OCR2A = 0;
    }else{
      OCR2A = flash_state?255:0;
      OCR2B = 0;
    }
  }else if(learning){
    wave_display = false;
    OCR2B = 0;
  }else{
    if(voct == 1023 || (voct == 0 && !lfo_mode)){
      wave_display = false;
    }
    if(voct == 1023){
      OCR2B = 255;
      OCR2A = 0;
    }else if(voct == 0 && !lfo_mode){
      OCR2A = 255;
      OCR2B = 0;
    }
  }

  bool frequency_changed = false;
  bool envelope_changed = false;

  int adc_centre = pgm_read_word(&(adc_centres[quantised_note]));
  int difference = abs(adc_centre - voct);
  if(difference >= 12){
    quantised_note = pgm_read_byte(&(quantised_notes[voct]));

    frequency_changed = true;
    
    #if debug
      Serial.println(quantised_note);
    #endif
  }

  if(abs(analogue - prev_analogue) >= tolerance){
    midi_note = analogue >> 3;

    frequency_changed = true;
    
    prev_analogue = analogue;
    #if debug
      Serial.print("midi_note:\t");
      Serial.print(midi_note);
      Serial.print("\tfreq:\t");
      Serial.print(mtof(midi_note));
      Serial.print("\tqn:\t");
      Serial.println(quantised_note);
    #endif
  }
  
  if(abs(analogue2 - prev_analogue2) >= tolerance){
    detune = (analogue2 >> 3) & B01111111; // constrain to 7 bits / 127

    frequency_changed = true;
    
    prev_analogue2 = analogue2;
    #if debug
      Serial.print("detune:\t");
      Serial.print(detune);
      Serial.print("\tpc:\t");
      Serial.println(1.25f + ((float)(detune - 63) * 0.01190f));
    #endif
  }
  
  if(abs(analogue3 - prev_analogue3) >= tolerance){
    cutoff = oneEighty((analogue3 >> 2) & B11111111); // constrain to 8 bits / 255
    prev_analogue3 = analogue3;
    #if debug
      Serial.print("env:\t");
      Serial.println(env);
    #endif
  }
  
  if(abs(analogue4 - prev_analogue4) >= tolerance){
    env = (analogue4 >> 3) & B01111111; // constrain to 7 bits / 127

    envelope_changed = true;
    
    prev_analogue4 = analogue4;
    #if debug
      Serial.print("env2:\t");
      Serial.println(env2);
    #endif
  }

  if(env == 127){ //always on
    gain = 255;
    filt_env = 45;
    env_octave = 0;
  }else{
    envelope.update();
    gain = envelope.next();
    envelope2.update();
    filt_env = envelope2.next();
    if(do_arp){
      //env_detune = (gain >> 1) & B11110000; // bitmask makes it more granular
      env_octave = (gain >> 5);
    }else{
      //env_detune = 0;
      env_octave = 0;
    }
  }

  if(lfo_mode){
      // filter control becomes attentuator / gain control
      gain = (cutoff * gain) >> 8;
  }
  
  if(env_octave != prev_env_octave
    || wave1 != prev_wave1
    || wave2 != prev_wave2
    || lfo_mode_changed
    || (lfo_mode && (do_arp != prev_do_arp))
    || midi_note_changed){
    frequency_changed = true;
    lfo_mode_changed = false;
    midi_note_changed = false;
  }
  
  if(frequency_changed){
    int total_note = midi_note + quantised_note + midi_input_note + (env_octave * 12) - ((env_octave>0)?36:0);
    if(total_note > 381){
      total_note-= 381;
    }else if(total_note > 254){
      total_note-= 254;
    }else if(total_note > 127){
      total_note-= 127;
    }
    if(lfo_mode){
      float set_freq = 0.0f;
      float second_freq = 0.0f;
      if(do_arp){
        set_freq = tempo_frequency;
        int integer_frequency = (detune >> 3);
        if(integer_frequency <= 8){
          integer_frequency = 9 - integer_frequency;
          second_freq = set_freq / integer_frequency;
        }else{
          integer_frequency = integer_frequency - 7;
          second_freq = set_freq * integer_frequency;
        }
        #if debug
          Serial.print("main f:\t");
          Serial.print(set_freq);
          Serial.print("\tint f:\t");
          Serial.print(integer_frequency);
          Serial.print("\tdiv/multed f:\t");
          Serial.println(second_freq);
        #endif
      }else{
        set_freq = mtof(total_note) * 0.01;
        second_freq = set_freq * pgm_read_float(&(detune_float[detune]));
      }

      // this code has to be duplicated because set_freq is declared as either float or int
      switch (wave1){
        case 0:
          aSqu.setFreq(set_freq);
          break;
        case 1:
          aSaw.setFreq(set_freq);
          break;
        case 2:
          aSin.setFreq(set_freq);
          break;
      }
      switch (wave2){
        case 0:
          aSqu2.setFreq(second_freq);
          break;
        case 1:
          aSaw2.setFreq(second_freq);
          break;
        case 2:
          aSin2.setFreq(second_freq);
          break;
      }
    }else{
      int set_freq = mtof(total_note);
      int second_freq = round(set_freq * pgm_read_float(&(detune_float[detune])));

      // this code has to be duplicated because set_freq is declared as either float or int
      switch (wave1){
        case 0:
          aSqu.setFreq(set_freq);
          break;
        case 1:
          aSaw.setFreq(set_freq);
          break;
        case 2:
          aSin.setFreq(set_freq);
          break;
      }
      switch (wave2){
        case 0:
          aSqu2.setFreq(second_freq);
          break;
        case 1:
          aSaw2.setFreq(second_freq);
          break;
        case 2:
          aSin2.setFreq(second_freq);
          break;
      }
    }
  }

  if(envelope_changed){
    if(env <= 63){ // attack env, gated
      unsigned int ms = (63 - env) << 4;
      if(lfo_mode){
        ms*= 10;
      }
      #if debug
        Serial.print("env attack ms:\t");
        Serial.println(ms);
      #endif
      envelope.setTimes(ms, 0, 0, 0); // attack envs have dead zones at start of knob
      envelope2.setTimes(round((float)ms*0.75f),round((float)ms*0.25f),0,0);
    }else{ // decay env, trigged
      unsigned int ms = (env - 64) << 4;
      if(lfo_mode){
        ms*= 10;
      }
      #if debug
        Serial.print("env decay ms:\t");
        Serial.println(ms);
      #endif
      envelope.setTimes(0, ms, 0, 0);
      envelope2.setTimes(round((float)ms*0.25f),round((float)ms*0.75f),0,0);
    }
  }
  
  byte set_cutoff = cutoff + filt_env;
  if(set_cutoff > 180){
    set_cutoff-= 180;
  }
  lpf.setCutoffFreqAndResonance(set_cutoff, res);

  bool clk_state = opt_read(clk_pin);
  bool clk_button_state = !opt_read(clk_button);
  if((clk_state && !clk_state_prev)
      || (clk_button_state && !clk_button_state_prev)){
    if(lfo_mode){
      switch (wave1){
        case 0:
          aSqu.setPhase(0);
          break;
        case 1:
          aSaw.setPhase(0);
          break;
        case 2:
          aSin.setPhase(0);
          break;
      }
      switch (wave2){
        case 0:
          aSqu2.setPhase(0);
          break;
        case 1:
          aSaw2.setPhase(0);
          break;
        case 2:
          aSin2.setPhase(0);
          break;
      }
    }
        
    envelope.noteOn(true);
    envelope2.noteOn(true);

    if(lfo_mode && do_arp){
      unsigned long period = micros() - last_clock;
      last_clock = micros();
      tempo_frequency = 1000.0f / ((float)period / 1000.0f);
      lfo_mode_changed = true;
    }
  }

  if(learning && clk_button_state){
    if(started_both_pressed == 0){
      started_both_pressed = millis();
      changing_mode = true;
      #if debug
        Serial.println("starting");
      #endif
    }else if((millis() - started_both_pressed) >= changing_mode_timeout){
      lfo_mode = !lfo_mode;
      lfo_mode_changed = true;
      changing_mode = false;
      started_both_pressed = 0;
      EEPROM.update(lfo_mode_addr, lfo_mode?1:0);
      #if debug
        Serial.print("changed to ");
        Serial.println(lfo_mode?"lfo":"vco");
      #endif
    }
  }else{
    started_both_pressed = 0;
    changing_mode = false;
  }

  if(!learning && prev_learning){
    EEPROM.write(midi_channel_addr, midi_channel);
    //test these additions..
    #if !debug
      MIDI.begin(midi_channel);
    #endif

    // learning button also functions as midi note reset.
    // If you press and release it without playing notes on a new channel, it won't change the midi channel
    // but will reset the played note to 0
    midi_input_note = 0;
    midi_note_changed = true;
  }else if(learning && !prev_learning){
    wave_display = false;
    OCR2A = 0;
    #if !debug
      MIDI.begin(MIDI_CHANNEL_OMNI);
    #endif
  }

  clk_state_prev = clk_state;
  clk_button_state_prev = clk_button_state;
  prev_learning = learning;
  prev_env_octave = env_octave;
  prev_wave1 = wave1;
  prev_wave2 = wave2;
  prev_mix = mix;
  prev_do_arp = do_arp;
  
  MIDI.read();
}

AudioOutput_t updateAudio(){

  int8_t asig1      = 0;
  int8_t asig2      = 0;
  int16_t mixed1    = 0;
  byte bitdepth     = 0;
  
  switch (wave1){
    case 0:
      asig1 = (int8_t)(aSqu.next());
      break;
    case 1:
      asig1 = (int8_t)(aSaw.next());
      break;
    case 2:
      asig1 = (int8_t)(aSin.next());
      break;
  }
  switch (wave2){
    case 0:
      asig2 = (int8_t)(aSqu2.next());
      break;
    case 1:
      asig2 = (int8_t)(aSaw2.next());
      break;
    case 2:
      asig2 = (int8_t)(aSin2.next());
      break;
  }
  switch (mix){
    case 0:
      mixed1     = (int)(asig1+asig2) >> 1;
      bitdepth   = 9;
      break;
    case 1:
      mixed1     = (int)(asig1^asig2);
      bitdepth   = 8;
      break;
    case 2:
      //mixed1     = (int)(asig1&asig2);
      mixed1     = (int)(asig1*asig2) >> 7;
      bitdepth   = 8;
      break;
  }

  int16_t gained    =  gain * mixed1;

  if(cutoff < 175 && !lfo_mode){ // save processing power / make sure unfiltered signal can be accessed
    int out_int = lpf.next(MonoOutput::fromNBit(16,gained));
    out_int = min(127,out_int);
    out = max(-128,out_int);
  }else{
    out = MonoOutput::fromNBit(16,gained);
  }

  if(wave_display){
    byte out_byte = (out + 128);
    OCR2A = out_byte;
    //OCR2B = 16-(out_byte >> 5);
    OCR2B = 255-out_byte;
  }

  return out;
}

void loop() {
  audioHook();
}


void HandleNoteOn(byte channel, byte note, byte velocity) {
  if(learning && !changing_mode){
    wave_display = false;
    OCR2A = 255;
    midi_channel = channel;
  }
  if(channel != midi_channel){
    return;
  }

  midi_input_note = note;
  midi_note_changed = true;
  envelope.noteOn(true);
  envelope2.noteOn(true);
}

void HandleNoteOff(byte channel, byte note, byte velocity) {
  if(learning){
    wave_display = false;
    OCR2A = 0;
    midi_channel = channel;
  }
  if(channel != midi_channel){
    return;
  }
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
