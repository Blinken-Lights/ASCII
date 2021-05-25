/*
 * - 8 bit sample & hold:
 *   - In 1: channel 1
 *   - In 2: channel 2
 *   - In 3: trigger
 *   - Out 1: channel 1
 *   - Out 2: channel 2
 * 
 * 
 * - LFO:
 *   - In 1: Speed
 *   - In 2: Range OR Skew OR Shape?
 *   - In 3: Sync
 *   - Out 1: Tri/Saw/Sine etc 
 *   - Out 2: Square
 *
 * - ADR:
 * 
 * 
 * 
 * - Quantiser:
 * 
 * 
 * 
 * 
 * - Drum:
 * 
 * 
 * 
 * 
 * - VCO:
 * 
 * 
 */

#define F_CPU 8000000  // This is used by delay.h library

#include <avr/io.h>        // Adds useful constants
#include <util/delay.h>    // Adds delay_ms and delay_us functions
#include <EEPROM.h>

#define EXT_RESET (1 << EXTRF)

int count = 0;
int max_apps = 5;

byte pwm1 = 0;
byte pwm2 = 0;

void setup() {

  count = EEPROM.read(0);
  if(count >= max_apps){
    count = 0;
  }

  // borrowed from https://sites.google.com/site/wayneholder/using-the-avr-reset-pin-as-an-input 'Wayne's Tinkering Page'
  if ((MCUSR & EXT_RESET) != 0) {
    // If External RESET then increment count
    MCUSR = MCUSR & ~EXT_RESET;
    count++;
    if(count >= max_apps){
      count = 0;
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

  OCR0A = 255;
  OCR0B = 255;

}

void loop() {

  /*
  // test sawtooth wave 
  OCR0A = pwm1;
  OCR0B = pwm1;
  pwm1++;

  _delay_ms(5);*/

  /*
  // test analogue inputs
  // 1 = PB2
  // 3 = PB3
  // 2 = PB4
  OCR0A = analogRead(1) >> 2; //PB2
  OCR0B = analogRead(3) >> 2; //PB3
  */

  //quantiser
  // to do - trigger.... should input 3 be normalled high too? - So quantisation is constant when nothing is plugged in
  //(void)analogRead(1);
  //_delay_ms(20);
  OCR0A = quantise(analogRead(1));
  //(void)analogRead(3);
  //_delay_ms(20);
  OCR0B = quantise(analogRead(3));

  //could do a single channel quantiser on IN3 where the other inputs are scale, and distance of CH2 note from the quantised one on CH1
  
}

byte quantise(int value){
  return round(min(60,round((float)((value >> 2) + 2) / 4.25)) * 4.25);
}

