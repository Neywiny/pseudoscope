// Testing interrupt-based analog reading
// ATMega328p

// Note, many macro values are defined in <avr/io.h> and
// <avr/interrupts.h>, which are included automatically by
// the Arduino interface

// Initialization


// for some reason this is having a hard time figuring itself out
#define UDR0 _SFR_MEM8(0xC6)

void setup(){
  pinMode(A5, INPUT);
  // clear ADLAR in ADMUX (0x7C) to right-adjust the result
  // ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits)
  ADMUX &= B11011111;
 
  // Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the
  // proper source (01)
  ADMUX |= B01000000;
 
  // Clear MUX3..0 in ADMUX (0x7C) in preparation for setting the analog
  // input
  ADMUX &= B11110000;
 
  // Set MUX3..0 in ADMUX (0x7C) to read from AD8 (Internal temp)
  // Do not set above 15! You will overrun other parts of ADMUX. A full
  // list of possible inputs is available in Table 24-4 of the ATMega328
  // datasheet
  ADMUX |= 5;
 
  // Set ADEN in ADCSRA (0x7A) to enable the ADC.
  // Note, this instruction takes 12 ADC clocks to execute
  //ADCSRA |= B10000000;
  // Set ADATE in ADCSRA (0x7A) to enable auto-triggering.
  ADCSRA |= B00100000;
 
  // Clear ADTS2..0 in ADCSRB (0x7B) to set trigger mode to free running.
  // This means that as soon as an ADC has finished, the next will be
  // immediately started.
  ADCSRB &= B11111000;
 
  // Set the Prescaler to 128 (16000KHz/128 = 125KHz)
  // Above 200KHz 10-bit results are not reliable.
  //ADCSRA |= B00000111;
 
  // Set ADIE in ADCSRA (0x7A) to enable the ADC interrupt.
  // Without this, the internal interrupt will not trigger.
  ADCSRA |= B00001000;
 
  // Enable global interrupts
  // AVR macro included in <avr/interrupts.h>, which the Arduino IDE
  // supplies by default.
  sei();
 
  Serial.begin(2000000, SERIAL_8N1);
  // Kick off the first ADC
  // Set ADSC in ADCSRA (0x7A) to start the ADC conversion
  ADCSRA |=B01000000;
  pinMode(LED_BUILTIN, OUTPUT);
}


// Processor loop
void loop(){

  // Whatever else you would normally have running in loop().
  digitalWrite(LED_BUILTIN, !(UCSR0A & (1<<UDRE0)));
 
}

// Interrupt service routine for the ADC completion
ISR(ADC_vect, ISR_NAKED){
  asm(
    "push r16\n\t"
    "1: lds r16, 0xC0\n\t"
    "sbrs r16,5\n\t"
    "rjmp 1b\n\t"
    "ldi r16, 0xA5\n\t" // r16 = 0xA5, the start of frame
    "sts 0xC6, r16\n\t" // UDr16 = r16 = 0xA5
    "1: lds r16, 0xC0\n\t"
    "sbrs r16,5\n\t"
    "rjmp 1b\n\t"
    "lds r16, 0x78\n\t" // r16 = ADCL
    "sts 0xC6, r16\n\t" // UDr16 = r16 = ADCL
    "1: lds r16, 0xC0\n\t"
    "sbrs r16,5\n\t"
    "rjmp 1b\n\t"
    "lds r16, 0x79\n\t" // r16 = ADCH
    "sts 0xC6, r16\n\t" // UDr16 = r16 = UDCH
    "pop r16\n\t"
    "reti\n\t"
  );
}
