typedef struct
{
    uint16_t interval;
    uint16_t last_tick;
    void (*func)(void);
} TaskType;

void recieve_input(void);
void switch_1(void);
void switch_2(void);

const int LED_TASKS = 1;
static TaskType g_tasks[] =
{
    {0, 0, recieve_input},
    // LED 1
    {500, 0, switch_1},
    // LED 2
    {500, 0, switch_2}
};

TaskType * task_config_ptr()
{
    return g_tasks;
}

int task_num_tasks()
{
    return sizeof(g_tasks) / sizeof(*g_tasks);
}

const int LED1 = 2;
const int LED2 = 3;

int led_pin[2];
unsigned led_timeout[2];
int led_state[2];
int led_off[2];

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
        led_off[i] = 0;
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
    TaskType *task_ptr;
    int num_tasks, i;

    unsigned timeout;
    unsigned tick;

    tick = (unsigned) millis();
    task_ptr = task_config_ptr();
    num_tasks = task_num_tasks();

    for(i = 0; i < num_tasks; i++) {
        if(task_ptr[i].interval == 0) {
            // Run continous tasks
            task_ptr[i].func();
        }
        else {
            // Run occasional tasks
            timeout =  task_ptr[i].interval;
            timeout += task_ptr[i].last_tick;

            if(overflow_cmp(tick, timeout) == 1) {
                task_ptr[i].func();
                task_ptr[i].last_tick = tick;
            }
        }
    }
}

void recieve_input()
{
    static int input;
    static int led_select;
    char peek;

    static int input_stage = 0;

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

    char input_complete;
    static char input_fail = 0;
    int c;
    if(Serial.available()) {
        c = Serial.read();

        if('0' <= c && c <= '9') {
            input *= 10;
            input += c - '0';
            input_complete = 0;
        }
        else if(c == '\n') {
            input_complete = 1;
        }
        else {
            input_fail = 1;
        }
    }

    if(input_complete) {

        if(input_fail) {
            input_stage--;
            Serial.println("Error: Input must only contain digits.");
        }
        else {
            Serial.println(input);
        }

        input_fail = 0;

        if(input_stage == STAGE_GET_LED) {
            led_select = input;
            if(1 > led_select || led_select > 2) {
                // Out of Range LEDs
                // Fail input
                Serial.println("Error: Invalid LED");
                input = 0;
                input_stage--;
            }
            else {
                input_stage++;
            }
        }
        if(input_stage == STAGE_GET_SPEED) {
            led_select--;

            if(input == 0) {
                led_off[led_select] = 1;
                led_state[led_select] = LOW;
                digitalWrite(led_pin[led_select], led_state[led_select]);
            }
            else {
                led_off[led_select] = 0;
            }

            TaskType *tasks;
            tasks = task_config_ptr();
            tasks[led_select+LED_TASKS].interval = input / 2;

            input_stage++;
        }

        // Repeat input stages endlessly
        if(input_stage > STAGE_GET_SPEED) {
            input_stage = STAGE_PROMPT_LED;
        }
    }
}

void switch_led(int x)
{
    x--; // The LED IDs are 1-indexed, while the arrays are 0 indexed.

    if(!led_off[x]) {
        // if LED is in the blinking state
        if(led_state[x] == LOW) {
            led_state[x] = HIGH;
        } else {
            led_state[x] = LOW;
        }
    }

    digitalWrite(led_pin[x], led_state[x]);
}

void switch_1()
{
   switch_led(1);
}

void switch_2()
{
    switch_led(2);
}