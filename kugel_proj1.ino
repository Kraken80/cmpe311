const int LED1 = 2;
const int LED2 = 3;

int led_pin[2];
unsigned led_speed[2];
unsigned led_timeout[2];
int led_state[2];
int led_off[2];

int input_stage = 0;

void setup() {
    int i;
    // put your setup code here, to run once:
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);

    Serial.begin(9600);
    Serial.setTimeout(0);

    led_pin[0] = LED1;
    led_pin[1] = LED2;

    for(i = 0; i < 2; i++) {
        led_state[i] = LOW;
        led_speed[i] = 500;
        led_off[i] = 0;
        //digitalWrite(led_pin[i], HIGH);
    }
}

int overflow_cmp(unsigned a, unsigned base)
{
    // Arduino longs are 32 bits, ints are 16 bits

    // Connect the number space in a loop,
    // the counterclockwise numbers are less,
    // the clockwise numbers are more,
    // the 'seam' is placed on the opposite side
    // of one of the numbers.

    unsigned seam;

    if(base == a) return 0;

    seam = base + 0x8000;

    if(seam > base) {
        if(a < base) return -1; // a < base
        if(a > seam) return -1; // a < base
        if(a > base) return 1; // a > base
    }
    else {
        // base > seam
        if(a > base) return 1; // a > base
        if(a < seam) return 1; // a > base
        if(a < base) return -1; // a < base
    }

    return 1;

}

enum {
    STAGE_PROMPT_LED = 0,
    STAGE_GET_LED,
    STAGE_PROMPT_SPEED,
    STAGE_GET_SPEED
};

enum {
    PROMPT = 0,
    GET = 1
};

void loop() {

    static int input;
    static int led_select;
    char peek;

    if(input_stage == STAGE_PROMPT_LED) {
        Serial.print("What LED? (1 or 2) ");
        input_stage++;
        input = 0;
    }

    if(input_stage == STAGE_PROMPT_SPEED) {
        Serial.print("What interval (in msec)? ");
        input_stage++;
        input = 0;
    }

    // Get Input
    // Get next input:
    // if 0-9 : accept
    // if '\n' : accept and spit out input
    // if else : disregard input

    char input_complete, input_fail;
    int c;
    if(Serial.available()) {
        c = Serial.read();

        if('0' <= c && c <= '9') {
            input *= 10;
            input += c - '0';
            input_complete = 0;
        }
        else if(c == '\n') {
            input_fail = 0;
            input_complete = 1;
        }
        else {
            input_fail = 1;
            input_complete = 1;
        }
    }

    if(input_complete) {

        Serial.println(input);

        if(input_fail) {
            input_stage--;
        }

        if(input_stage == STAGE_GET_LED) {
            led_select = input;
            if(1 > led_select || led_select > 2) {
                // Out of Range LEDs
                // Fail input
                input_stage--;
            }
        }
        if(input_stage == STAGE_GET_SPEED) {
            led_select--;

            if(input == 0) {
                led_off[led_select] = 1;
            }
            else {
                led_off[led_select] = 0;
            }

            led_speed[led_select] = input / 2;
        }

        input_stage++;

        // Repeat input stages endlessly
        if(input_stage > STAGE_GET_SPEED) {
            input_stage = STAGE_PROMPT_LED;
        }
    }

    // Change LED States
    unsigned time;

    //time = (unsigned) micros();
    time = (unsigned) millis();

    int i, compare;

    for(i = 0; i < 2; i++) {
        if(led_off[i]) {
            led_timeout[i] = time;
            // Keep off LEDs up to date for when they are turned back on
            if(led_state[i]) {
                led_state[i] = LOW;
                compare = 1;
            }
            else {
                compare = 0;
            }
        }
        else if(!led_off[i]) {
            compare = overflow_cmp(time, led_timeout[i]);

            /*
             *      if(time > led_timeout[i]) {
             *        compare = 1;
        }
        else {
            compare = 0;
        }
        */
        }

        //Serial.print(time);
        //Serial.print(" ");
        //Serial.println(led_timeout[i]);

        if(compare == 1) {
            // A > B
            led_timeout[i] += led_speed[i];

            if(!led_off[i]) {
                // if LED is in the blinking state
                if(led_state[i] == LOW) {
                    led_state[i] = HIGH;
                } else {
                    led_state[i] = LOW;
                }
            }

            digitalWrite(led_pin[i], led_state[i]);
        }
    }

}
