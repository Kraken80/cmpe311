//#include <avr/interrupt.h>
#ifndef MILLIS_H
#define MILLIS_H

typedef unsigned char byte_t;

void ext_io_read(byte_t addr, byte_t *ret_h)
{
  unsigned volatile char ret;

  __asm__ __volatile__ (
    "eor r27,r27\n" // Clear X high byte
    "mov r26, %1\n" // Fill X low byte with argument
    "ld %0, X\n" // Load %0 with data at location X
    : "=r" (ret) // Return
    : "r" (addr) // Substitutions
    : "r26", "r27" // Clobber
  );

  *ret_h = ret;
}

void ext_io_write(byte_t addr, byte_t val)
{
  __asm__ __volatile__ (
    "clr r27\n" // Clear X high byte
    "mov r26, %0\n"
    "st X, %1\n" // Load %0 with data at location X
    : // Return
    : "r" (addr), "r" (val) // Substitutions
    : "r26", "r27" // Clobber
  );
}

#define RESET 0
#define SET 1
void ext_io_bit(byte_t addr, unsigned mask, unsigned mode)
{
  byte_t x;

  ext_io_read(addr, &x);

  if(mode == RESET) {
    x &= ~mask;
  }
  if(mode == SET) {
    x |= mask;
  }

  ext_io_write(addr, x);
}

// power saving settings
#define IO_PRR 0x64
// Disables timer 2 if set
#define PRTIM2 (1 << 6)

#define IO_TCCR2A 0xB0
#define IO_TCCR2B 0xB1
#define IO_OCR2A 0xB3

#define WGM20 (1<<0)
#define WGM21 (1<<1)
#define WGM22 (1<<3)

#define CS20 (1<<0)
#define CS21 (1<<1)
#define CS22 (1<<2)

#define IO_TIMSK2 0x70
#define TOIE2 (1<<0)

#define IO_TCNT2 0xB2

// Because IN, OUT I/O instructions must be an immediate argument,
// a macro is used to convert the addr into a constant.
#define io_read(addr, ret) \
{ \
  __asm__ __volatile__ ("in %0, %1\n": "=r" (ret) : "I" (addr)); \
}

#define io_write(addr, val) \
{ \
  __asm__ __volatile__ ("out %1, %0\n": :"r" (val), "I" (addr)); \
}

// global time variable
volatile int _time_f = 0;
unsigned long _millis = 0;
int _fail = 0;

unsigned long mymillis() {
  return _millis;
}

#define SYSTEM_CLOCK_HZ 16000000L // Depends on crystal
#define ISR_PERIOD 64
#define ISR_HZ 15625L // SYSTEM_CLOCK_HZ / (2 * CLOCK_PRESCALE * ISR_PERIOD);

// 1ms = MAX/ADV = 1000/ISR_HZ
#define MAX 125
#define ADV 2
// I/O clock = System Clock

void mymillis_init(void)
{
  
  byte_t tccra = 0, tccrb = 0, ocra = 0;

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  // We use Timer 2 since Arduino already uses Timer 0 for its millis()

  cli();

  // Disable power saving setting that may turn off Timer 2
  ext_io_bit(IO_PRR, PRTIM2, RESET);

  // Set up Timer 2 Control registers
  tccra = 0;
  tccrb = 0;

  // Set Normal mode (Mode 0) to Timer 2
  tccra &= ~WGM20;
  tccra &= ~WGM21;
  tccrb &= ~WGM22;

  // Set CTC mode (Mode 2) to Timer 2
  /*
  tccra &= ~WGM20;
  tccra |= WGM21;
  tccrb &= ~WGM22;
  */

  // Set Compare Register to X to Timer 2
  ocra = ISR_PERIOD;

  // Set clock source to no prescaling (Mode 1) to Timer 2
  tccrb |= CS20;
  tccrb &= ~CS21;
  tccrb &= ~CS22;

  ext_io_write(IO_TCCR2A, tccra);
  ext_io_write(IO_TCCR2B, tccrb);
  ext_io_write(IO_OCR2A, ocra);

  // SREG I-bit, TOV0 automatically control interrupts
  // Enable Timer2 Overflow Interrupt
  ext_io_bit(IO_TIMSK2, TOIE2, SET);

  sei();
  
  // debug prints
  //Serial.begin(9600);
}

/*
// Testing loop for millis.h
void loop(void)
{
  static byte_t state = 0;

  if(_millis > 1000) {
    _millis = 0;
    state = !state;
    digitalWrite(LED_BUILTIN, state);
  }
  else {
    Serial.print(_time_f);
    Serial.print("\t");
    Serial.println(_millis);
  }
}
*/

/*
ISR(BADISR_vect)
{
  _fail = 1;
  reti();
}
*/

ISR(TIMER2_OVF_vect)
//ISR(TIMER2_COMPA_vect)
{

  _time_f += ADV;
  if(_time_f > MAX) {
    _millis++;
    _time_f -= MAX;
  }
  
  ext_io_write(IO_OCR2A, ISR_PERIOD);
}

#endif
