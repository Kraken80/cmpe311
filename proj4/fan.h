
typedef unsigned char byte_t;

#define DDRD (* (byte_t*)0x2A)
#define DDD6 (1 << 6)

#define OCR0A (* (byte_t*)0x47)

#define TIMSK0 (* (byte_t*)0x6E)

#define TOIE0 (1 << 0)
#define OCIE0A (1 << 1)
#define OCIE0B (1 << 2)

#define TCCR0A (* (byte_t*)0x44)
#define TCCR0B (* (byte_t*)0x45)

#define CS00 (1 << 0)
#define CS01 (1 << 1)
#define CS02 (1 << 2)

#define COM0A1 (1 << 7)
#define COM0A0 (1 << 6)
#define COM0B1 (1 << 5)
#define COM0B0 (1 << 4)

#define WGM00 (1 << 0)
#define WGM01 (1 << 1)
#define WGM02 (1 << 3)

#define PRR (* (byte_t*)0x64)

#define PRTIM0 (1 << 5)
#define PRTIM2 (1 << 6)

void fan_init()
{
    // Disable Timer 0 power saving
    PRR &= ~PRTIM0;

    // Set Timer 0 to PWM mode (Mode 1)
    // Mode 5 DOES NOT WORK, because TOP = COMPARE
    // WGM02:0 = 1
    // OCR0A = PWM level
    // OCnx Interrupt Flag Set on each PWM change
    TCCR0A |= WGM00;
    TCCR0A &= ~WGM01;
    TCCR0B &= ~WGM02;

    //Set Clock Select to CLK/256
    // CS00:2 = 4
    TCCR0B &= ~CS00;
    TCCR0B &= ~CS01;
    TCCR0B |= CS02;

    //Set PWM Level
    OCR0A = 0xFF; // Turn off initially
    //OCR0A = 0xFF; // Always on

    // Set OC0A pin to Output Direction using the DDR.
    DDRD |= DDD6;

    // Set Timer Compare Output Mode to
    // Inverted OC0A mode
    // (OC0A: AIN0 which is Arduino Digital Pin 6)
    // COM0A1:0 = 2
    // COM0B1:0 = 2
    TCCR0A &= ~COM0A0;
    TCCR0A |= COM0A1;
    TCCR0A &= ~COM0B0;
    TCCR0A |= COM0B1;

    // Enable interrupt (optional)
    // TIMSK0 |= OCIE0A;
    // TIMSK0 |= OCIE0B;
}
