byte inputTypeSelectorPin = 0;
bool inputType = false; // true = 2 inputs plus CV, false = 3 inputs and no CV
//bool prevInputType = false;

int maxLogicModes = 5;
byte logicModes[] = { 0, 0, 0, 0 };

byte analoguePins[] = { A0, A1, A2, A3, A4, A5, A6, A7 };
int analogueReadings[] = { -1, -1, -1, -1, -1, -1, -1, -1 };
int jitterThreshold = 14;
byte readAnaloguePin = 0;

// Just for reference - the pin manipulation is done by optimised routines:
byte inputPins[4][2] = {
  { 13, 12 },
  { 10, 9 },
  { 7, 6 },
  { 4, 3 },
};

byte outputPins[] = {
  11,
  8,
  5,
  2
};

void setup() {

  //Serial.begin(250000);

  pinMode(inputTypeSelectorPin, INPUT_PULLUP);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  for(int i = 0; i < 4; i++){
    for(int j = 0; j < 2; j++){
      pinMode(inputPins[i][j], INPUT);
    }
    pinMode(outputPins[i], OUTPUT);
  }

  // TIMER 0 for interrupt frequency 50000 Hz:
  cli(); // stop interrupts
  TCCR0A = 0; // set entire TCCR0A register to 0
  TCCR0B = 0; // same for TCCR0B
  TCNT0  = 0; // initialize counter value to 0
  // set compare match register for 50000 Hz increments
  OCR0A = 39; // = 16000000 / (8 * 50000) - 1 (must be <256)
  // turn on CTC mode
  TCCR0B |= (1 << WGM01);
  // Set CS02, CS01 and CS00 bits for 8 prescaler
  TCCR0B |= (0 << CS02) | (1 << CS01) | (0 << CS00);
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  sei(); // allow interrupts

}

ISR(TIMER0_COMPA_vect){

  // 0 = AND, 1 = OR, 2 = XOR, 3 = NAND, 4 = NOR, 5 = XNOR

  if(inputType){

    switch (logicModes[0]){
      case 0:
        bitWrite(PORTB, 3, bitRead(PINB, 5) & bitRead(PINB, 4));
      break;
      case 1:
        bitWrite(PORTB, 3, bitRead(PINB, 5) | bitRead(PINB, 4));
      break;
      case 2:
        bitWrite(PORTB, 3, bitRead(PINB, 5) ^ bitRead(PINB, 4));
      break;
      case 3:
        bitWrite(PORTB, 3, !(bitRead(PINB, 5) & bitRead(PINB, 4)));
      break;
      case 4:
        bitWrite(PORTB, 3, !(bitRead(PINB, 5) | bitRead(PINB, 4)));
      break;
      case 5:
        bitWrite(PORTB, 3, !(bitRead(PINB, 5) ^ bitRead(PINB, 4)));
      break;
    }

    switch (logicModes[1]){
      case 0:
        bitWrite(PORTB, 0, bitRead(PINB, 1) & bitRead(PINB, 2));
      break;
      case 1:
        bitWrite(PORTB, 0, bitRead(PINB, 1) | bitRead(PINB, 2));
      break;
      case 2:
        bitWrite(PORTB, 0, bitRead(PINB, 1) ^ bitRead(PINB, 2));
      break;
      case 3:
        bitWrite(PORTB, 0, !(bitRead(PINB, 1) & bitRead(PINB, 2)));
      break;
      case 4:
        bitWrite(PORTB, 0, !(bitRead(PINB, 1) | bitRead(PINB, 2)));
      break;
      case 5:
        bitWrite(PORTB, 0, !(bitRead(PINB, 1) ^ bitRead(PINB, 2)));
      break;
    }

    switch (logicModes[2]){
      case 0:
        bitWrite(PORTD, 5, bitRead(PIND, 6) & bitRead(PIND, 7));
      break;
      case 1:
        bitWrite(PORTD, 5, bitRead(PIND, 6) | bitRead(PIND, 7));
      break;
      case 2:
        bitWrite(PORTD, 5, bitRead(PIND, 6) ^ bitRead(PIND, 7));
      break;
      case 3:
        bitWrite(PORTD, 5, !(bitRead(PIND, 6) & bitRead(PIND, 7)));
      break;
      case 4:
        bitWrite(PORTD, 5, !(bitRead(PIND, 6) | bitRead(PIND, 7)));
      break;
      case 5:
        bitWrite(PORTD, 5, !(bitRead(PIND, 6) ^ bitRead(PIND, 7)));
      break;
    }

    switch (logicModes[3]){
      case 0:
        bitWrite(PORTD, 2, bitRead(PIND, 3) & bitRead(PIND, 4));
      break;
      case 1:
        bitWrite(PORTD, 2, bitRead(PIND, 3) | bitRead(PIND, 4));
      break;
      case 2:
        bitWrite(PORTD, 2, bitRead(PIND, 3) ^ bitRead(PIND, 4));
      break;
      case 3:
        bitWrite(PORTD, 2, !(bitRead(PIND, 3) & bitRead(PIND, 4)));
      break;
      case 4:
        bitWrite(PORTD, 2, !(bitRead(PIND, 3) | bitRead(PIND, 4)));
      break;
      case 5:
        bitWrite(PORTD, 2, !(bitRead(PIND, 3) ^ bitRead(PIND, 4)));
      break;
    }

  }else{

    switch (logicModes[0]){
      case 0:
        bitWrite(PORTB, 3, bitRead(PINB, 5) & bitRead(PINB, 4) & bitRead(PINC, 0));
      break;
      case 1:
        bitWrite(PORTB, 3, bitRead(PINB, 5) | bitRead(PINB, 4) | bitRead(PINC, 0));
      break;
      case 2:
        bitWrite(PORTB, 3, bitRead(PINB, 5) ^ bitRead(PINB, 4) ^ bitRead(PINC, 0));
      break;
      case 3:
        bitWrite(PORTB, 3, !(bitRead(PINB, 5) & bitRead(PINB, 4) & bitRead(PINC, 0)));
      break;
      case 4:
        bitWrite(PORTB, 3, !(bitRead(PINB, 5) | bitRead(PINB, 4) | bitRead(PINC, 0)));
      break;
      case 5:
        bitWrite(PORTB, 3, !(bitRead(PINB, 5) ^ bitRead(PINB, 4) ^ bitRead(PINC, 0)));
      break;
    }

    switch (logicModes[1]){
      case 0:
        bitWrite(PORTB, 0, bitRead(PINB, 1) & bitRead(PINB, 2) & bitRead(PINC, 1));
      break;
      case 1:
        bitWrite(PORTB, 0, bitRead(PINB, 1) | bitRead(PINB, 2) | bitRead(PINC, 1));
      break;
      case 2:
        bitWrite(PORTB, 0, bitRead(PINB, 1) ^ bitRead(PINB, 2) ^ bitRead(PINC, 1));
      break;
      case 3:
        bitWrite(PORTB, 0, !(bitRead(PINB, 1) & bitRead(PINB, 2) & bitRead(PINC, 1)));
      break;
      case 4:
        bitWrite(PORTB, 0, !(bitRead(PINB, 1) | bitRead(PINB, 2) | bitRead(PINC, 1)));
      break;
      case 5:
        bitWrite(PORTB, 0, !(bitRead(PINB, 1) ^ bitRead(PINB, 2) ^ bitRead(PINC, 1)));
      break;
    }

    switch (logicModes[2]){
      case 0:
        bitWrite(PORTD, 5, bitRead(PIND, 6) & bitRead(PIND, 7) & bitRead(PINC, 2));
      break;
      case 1:
        bitWrite(PORTD, 5, bitRead(PIND, 6) | bitRead(PIND, 7) | bitRead(PINC, 2));
      break;
      case 2:
        bitWrite(PORTD, 5, bitRead(PIND, 6) ^ bitRead(PIND, 7) ^ bitRead(PINC, 2));
      break;
      case 3:
        bitWrite(PORTD, 5, !(bitRead(PIND, 6) & bitRead(PIND, 7) & bitRead(PINC, 2)));
      break;
      case 4:
        bitWrite(PORTD, 5, !(bitRead(PIND, 6) | bitRead(PIND, 7) | bitRead(PINC, 2)));
      break;
      case 5:
        bitWrite(PORTD, 5, !(bitRead(PIND, 6) ^ bitRead(PIND, 7) ^ bitRead(PINC, 2)));
      break;
    }

    switch (logicModes[3]){
      case 0:
        bitWrite(PORTD, 2, bitRead(PIND, 3) & bitRead(PIND, 4) & bitRead(PINC, 3));
      break;
      case 1:
        bitWrite(PORTD, 2, bitRead(PIND, 3) | bitRead(PIND, 4) | bitRead(PINC, 3));
      break;
      case 2:
        bitWrite(PORTD, 2, bitRead(PIND, 3) ^ bitRead(PIND, 4) ^ bitRead(PINC, 3));
      break;
      case 3:
        bitWrite(PORTD, 2, !(bitRead(PIND, 3) & bitRead(PIND, 4) & bitRead(PINC, 3)));
      break;
      case 4:
        bitWrite(PORTD, 2, !(bitRead(PIND, 3) | bitRead(PIND, 4) | bitRead(PINC, 3)));
      break;
      case 5:
        bitWrite(PORTD, 2, !(bitRead(PIND, 3) ^ bitRead(PIND, 4) ^ bitRead(PINC, 3)));
      break;
    }

  }

}

void loop() {

  int analogueReading = analogRead(analoguePins[readAnaloguePin]);
  if(abs(analogueReading - analogueReadings[readAnaloguePin]) > jitterThreshold){
    analogueReadings[readAnaloguePin] = analogueReading;
    readAnaloguePin = readAnaloguePin % 4;
    byte newLogicMode = 0;
    if(!inputType){
      newLogicMode = round(analogueReadings[readAnaloguePin + 4] * 0.00488758553f);
    }else{
      newLogicMode = round((analogueReadings[readAnaloguePin] + analogueReadings[readAnaloguePin + 4]) * 0.00488758553f);
    }
    newLogicMode = newLogicMode % (maxLogicModes + 1);
    logicModes[readAnaloguePin] = newLogicMode;
    /*Serial.print(analogueReadings[readAnaloguePin]);
    Serial.print("\t");
    Serial.print(readAnaloguePin);
    Serial.print("\t");
    Serial.println(logicModes[readAnaloguePin]);*/
  }
  readAnaloguePin++;
  if(readAnaloguePin >= 8){
    readAnaloguePin = 0;
  }

  inputType = opt_read(inputTypeSelectorPin);

  //if(prevInputType != inputType){
  // Serial.println(inputType ? "2 inputs" : "3 inputs");
  //}
  //prevInputType = inputType;

}

bool opt_read(byte pin) {
  if (pin >= 14) {
    pin -= 14;
    return bitRead(PINC, pin);
  } else if (pin >= 8) {
    pin -= 8;
    return bitRead(PINB, pin);
  } else {
    return bitRead(PIND, pin);
  }
}

void opt_write(byte pin, bool val) {
  if (pin >= 14) {
    pin -= 14;
    bitWrite(PORTC, pin, val);
  } else if (pin >= 8) {
    pin -= 8;
    bitWrite(PORTB, pin, val);
  } else {
    bitWrite(PORTD, pin, val);
  }
}

void opt_mode(byte pin, byte val) {
  if (val == 2) {
    opt_write(pin, HIGH);
    val = 0;
  }
  if (pin >= 14) {
    pin -= 14;
    bitWrite(DDRC, pin, val);
  } else if (pin >= 8) {
    pin -= 8;
    bitWrite(DDRB, pin, val);
  } else {
    bitWrite(DDRD, pin, val);
  }
}
