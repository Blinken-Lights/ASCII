
// Polyrhythms Module

int clock_pin = 2; //interrupt pin
int reset_pin = 3; //interrupt pin
int binary_or_bases_pin = 4;
int reset_button = 5;

int clock_led = 6;
int reset_led = 7;

bool prev_reset_state = false;

bool binary_or_bases = true;

bool run_multiplier = false;

byte binary_count = 0;
int base_count = 0;

const byte port_states[] PROGMEM = {0,1,2,5,8,19,32,5,2,9,0,23,0,33,10,5,0,19,0,13,34,1,0,23,8,1,2,37,0,27,0,5,2,1,40,23,0,1,2,13,0,51,0,5,10,1,0,23,32,9,2,5,0,19,8,37,2,1,0,31,0,1,34,5,8,19,0,5,2,41,0,23,0,1,10,5,32,19,0,13,2,1,0,55,8,1,2,5,0,27,32,5,2,1,8,23,0,33,2,13,0,19,0,5,42,1,0,23,0,9,2,37,0,19,8,5,2,1,32,31,0,1,2,5,8,51,0,5,2,9,0,23,32,1,10,5,0,19,0,45,2,1,0,23,8,1,34,5,0,27,0,5,2,33,8,23,0,1,2,13,32,19,0,5,10,1,0,55,0,9,2,5,0,19,40,5,2,1,0,31,0,33,2,5,8,19,0,5,34,9,0,23,0,1,10,37,0,19,0,13,2,1,32,23,8,1,2,5,0,59,0,5,2,1,8,23,32,1,2,13,0,19,0,37,10,1,0,23,0,9,34,5,0,19,8,5,2,33,0,31,0,1,2,5,40,19,0,5,2,9,0,55,0,1,10,5,0,19,32,13,2,1,0,23,8,33,2,5,0,27,0,5,34,1,8,23,0,1,2,45,0,19,0,5,10,1,32,23,0,9,2,5,0,51,8,5,2,1,0,31,32,1,2,5,8,19,0,37,2,9,0,23,0,1,42,5,0,19,0,13,2,33,0,23,8,1,2,5,32,27,0,5,2,1,8,55,0,1,2,13,0,19,32,5,10,1,0,23,0,41,2,5,0,19,8,5,34,1,0,31,0,1,2,37,8,19,0,5,2,9,32,23,0,1,10,5,0,51,0,13,2,1,0,23,40,1,2,5,0,27,0,37,2,1,8,23,0,1,34,13,0,19,0,5,10,33,0,23,0,9,2,5,32,19,8,5,2,1,0,63};

unsigned long period_counter = 0;
unsigned long period = 0;
unsigned long current_millis = 0;
unsigned long period_interrupts = 0;
unsigned long half_period_interrupts = 0;
unsigned long quarter_period_interrupts = 0;
unsigned long eigth_period_interrupts = 0;
unsigned long sixteenth_period_interrupts = 0;
unsigned long thirtysecondth_period_interrupts = 0;
unsigned long sixtyfourth_period_interrupts = 0;
unsigned long same_period = 0;
unsigned long half_period = 0;
unsigned long quarter_period = 0;
unsigned long eigth_period = 0;
unsigned long sixteenth_period = 0;
unsigned long thirtysecondth_period = 0;
unsigned long sixtyfourth_period = 0;

unsigned long third_period_interrupts = 0;
unsigned long fifth_period_interrupts = 0;
unsigned long sixth_period_interrupts = 0;
unsigned long seventh_period_interrupts = 0;
unsigned long third_period = 0;
unsigned long fifth_period = 0;
unsigned long sixth_period = 0;
unsigned long seventh_period = 0;

byte multiple_binary = 0;

void setup() {
    pinMode(clock_pin, INPUT);
    pinMode(reset_pin, INPUT);
    pinMode(reset_button, INPUT_PULLUP);
    pinMode(binary_or_bases_pin, INPUT_PULLUP);

    pinMode(clock_led, OUTPUT);
    pinMode(reset_led, OUTPUT);

    //PORTB
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);

    //PORTC
    pinMode(A0, OUTPUT);
    pinMode(A1, OUTPUT);
    pinMode(A2, OUTPUT);
    pinMode(A3, OUTPUT);
    pinMode(A4, OUTPUT);
    pinMode(A5, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(clock_pin), clock_function, RISING);
    attachInterrupt(digitalPinToInterrupt(reset_pin), reset_function, RISING);

    cli();
  
    TCCR1A = 0;// set entire TCCR2A register to 0
    TCCR1B = 0;// same for TCCR2B
    TCNT1  = 0;//initialize counter value to 0
    OCR1A = 1999; //1000Hz: [ 16,000,000Hz / (prescaler * desired interrupt frequency) ] - 1 (must be <65536)
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Prescaler of 8
    TCCR1B |= (1 << CS11);   
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);
  
    sei();

    set_port_state();
}

//fire an interrupt every millisecond
//turn off pins on every half, quarter, eigth period
ISR(TIMER1_COMPA_vect){

    if(!run_multiplier){
        return false;
    }

    if(binary_or_bases){
        if(period_interrupts == 0
            || period_interrupts == same_period){
            period_interrupts = 0;
            half_period_interrupts = 0;
            quarter_period_interrupts = 0;
            eigth_period_interrupts = 0;
            sixteenth_period_interrupts = 0;
            thirtysecondth_period_interrupts = 0;
            sixtyfourth_period_interrupts = 0;
            multiple_binary = 255;
        }else if(period_interrupts == half_period){
            half_period_interrupts = 0;
            quarter_period_interrupts = 0;
            eigth_period_interrupts = 0;
            sixteenth_period_interrupts = 0;
            thirtysecondth_period_interrupts = 0;
            sixtyfourth_period_interrupts = 0;
            multiple_binary--;
        }else if(half_period_interrupts == quarter_period){
            quarter_period_interrupts = 0;
            eigth_period_interrupts = 0;
            sixteenth_period_interrupts = 0;
            thirtysecondth_period_interrupts = 0;
            sixtyfourth_period_interrupts = 0;
            multiple_binary--;
        }else if(quarter_period_interrupts == eigth_period){
            eigth_period_interrupts = 0;
            sixteenth_period_interrupts = 0;
            thirtysecondth_period_interrupts = 0;
            sixtyfourth_period_interrupts = 0;
            multiple_binary--;
        }else if(eigth_period_interrupts == sixteenth_period){
            sixteenth_period_interrupts = 0;
            thirtysecondth_period_interrupts = 0;
            sixtyfourth_period_interrupts = 0;
            multiple_binary--;
        }else if(sixteenth_period_interrupts == thirtysecondth_period){
            thirtysecondth_period_interrupts = 0;
            sixtyfourth_period_interrupts = 0;
            multiple_binary--;
        }else if(thirtysecondth_period_interrupts == sixtyfourth_period){
            sixtyfourth_period_interrupts = 0;
            multiple_binary--;
        }
    }else{

        if(period_interrupts == 0){
            half_period_interrupts = 0;
            quarter_period_interrupts = 0;
            third_period_interrupts = 0;
            fifth_period_interrupts = 0;
            sixth_period_interrupts = 0;
            seventh_period_interrupts = 0;
            multiple_binary = 255;
        }else{

            if(half_period_interrupts == half_period){
                half_period_interrupts = 0;
                multiple_binary ^= (1 << 5);
            }else if(third_period_interrupts == third_period){
                third_period_interrupts = 0;
                multiple_binary ^= (1 << 4);
            }
            if(quarter_period_interrupts == quarter_period){
                quarter_period_interrupts = 0;
                multiple_binary ^= (1 << 3);
            }else if(fifth_period_interrupts == fifth_period){
                fifth_period_interrupts = 0;
                multiple_binary ^= (1 << 2);
            }
            if(sixth_period_interrupts == sixth_period){
                sixth_period_interrupts = 0;
                multiple_binary ^= (1 << 1);
            }else if(seventh_period_interrupts == seventh_period){
                seventh_period_interrupts = 0;
                multiple_binary ^= (1 << 0);
            }
            
        }
    }

    PORTC = multiple_binary;

    if(binary_or_bases){
        period_interrupts++;
        half_period_interrupts++;
        quarter_period_interrupts++;
        eigth_period_interrupts++;
        sixteenth_period_interrupts++;
        thirtysecondth_period_interrupts++;
        sixtyfourth_period_interrupts++;
    }else{
        period_interrupts++;
        half_period_interrupts++;
        quarter_period_interrupts++;
        third_period_interrupts++;
        fifth_period_interrupts++;
        sixth_period_interrupts++;
        seventh_period_interrupts++;
    }

    if(period_interrupts==period){
        run_multiplier = false; 
    }
    
}

void set_port_state(){
    if(binary_or_bases){
        PORTB = binary_count;
    }else{
        PORTB = pgm_read_byte_near(port_states + base_count);
    }
}

void clock_function(){
    current_millis = millis();
    period = current_millis - period_counter;
    period_counter = current_millis;
    binary_count++;
    base_count++;
    if(binary_count > 63){
        binary_count = 0;
    }
    if(base_count > 419){
        base_count = 0;
    }
    set_port_state();

    period_interrupts = 0;
    same_period = period >> 1;
    
    multiple_binary = 255;
    run_multiplier = true;

    if(binary_or_bases){
        half_period = same_period >> 1;
        quarter_period = half_period >> 1;
        eigth_period = quarter_period >> 1;
        sixteenth_period = eigth_period >> 1;
        thirtysecondth_period = sixteenth_period >> 1;
        sixtyfourth_period = thirtysecondth_period >> 1;
    }else{
        half_period = same_period >> 1;
        third_period = period / 6;
        quarter_period = half_period >> 1;
        fifth_period = period / 10;
        sixth_period = third_period >> 1;
        seventh_period = period / 14;
    }
}

void reset_function(){
    multiple_binary = 0;
    binary_count = 0;
    base_count = 0;
    period_interrupts = 0;
    set_port_state();
}

void loop() {

    binary_or_bases = !opt_read(binary_or_bases_pin);

    opt_write(clock_led, opt_read(clock_pin));
    opt_write(reset_led, opt_read(reset_pin));

    bool reset_state = !opt_read(reset_button);

    if(reset_state && reset_state != prev_reset_state){
        reset_function();
    }
    prev_reset_state = reset_state;
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
