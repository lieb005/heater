/*
 * File:   main.c
 * Author: talia
 *
 * Created on October 9, 2016, 3:47 PM
 */

// TODO: Use Capture module for IR. Need to change FOSC
// so that it can have the right baud (36k? 2*36k?)
// Also need to free either timer 1 or timer 3 for usage for the CCP.
// Also need to move stuff off of CCP1 or CCP2 pin to allow us to connect to it.

#include <math.h>
#include <stdint.h>
#include "main.h"
//#include "rc5.h"
#include "disp.h"

#include <xc.h>


#pragma config FOSC = INTOSCIO_EC
#pragma config MCLRE = 1 // Dedicate pin to MCLR
#pragma config PBADEN = 0 // Analog ins disabled on statup
#pragma config WDT = 0 // Let SWDTEN decide
#pragma config WDTPS = 32768 // Set postscale to 1:32768
#pragma config PWRT = 1 // Start up timer
#pragma config LPT1OSC = 0 // High power Timer1
#pragma config DEBUG = 0 // Inverted logic?
#pragma config LVP = 0 // High voltage programming

// FOSC before div 4 but after divider
// 8MHz so we can have most precision
#define _XTAL_FREQ 8000000UL

void setup();
void set_LEDs(unsigned char mask);
char sample_inputs();
float get_temp();
// Save state to eeprom
void ee_save_state();
// Read state from eeprom
void ee_read_state();
// To call if overheat or any other number of reasons
void error(char code);
//unsigned short num_to_disp(char num);
void buzzer(char duration);

void ee_write_byte(unsigned char addr, unsigned char data);
unsigned char ee_read_byte(unsigned char addr);

void set_key_led_disp_tris(unsigned char dir);

// Target temp
char temp_want;
// Number of seconds since midnight
long time;
// What are the hours bounding the day/night
// Set by user
char day_hour, night_hour;
// Temp for day time and night time
char day_temp, night_temp;
// Is the timer on?
bit timer = 0;
//  Are we in *F? 0 = *C
bit unit = 0;
// What number do the up/down buttons modify?
char *cur_number = &temp_want;
// Our global state
global_state state = _OFF;
// Only use to control whether to use heat1 or heat_both
heat_state heat_mode = _HEAT_BOTH;
// What do we want the output to actually be?
heat_state heat_cur;
// Do we heat, cool, or both?
heat_or_cool climate_control = _HEAT;
// What should we display?
disp_modes disp_mode = _DISP_OFF;

// Variable to track which part of timer we are setting
timer_set_state timer_are_setting;
// Current Temperature
float temp_cur;
// Variable of what's on the display.  BIT7 is the on/off bit
unsigned char disp[2];
// Both countdown variables to control blink/buzz duration
// They are changed every 4Hz, so we need to divide blink by 2?
unsigned char buzz = 0, blink = 0;
// Used for blinking
#define BLINK_DIV 2
#define BLINK (blink / BLINK_DIV)

bit timer_led = 0, power_led = 0;
// Use like a countdown, but when we want fan on, feed this like a watchdog
// Lets the fan blow out all the hot air if the heat turns off
char fan_timeout = 0;

void main(void)
{
    setup();
    ee_read_state();
    while (1)
    {
        // Feed the watchdog :)
        asm("clrwdt");
        // Save the state here so we don't forget it on restart
        ee_save_state();

        switch (state)
        {
            // We shouldn't be doing anything if we're off
        case _OFF:
            // Set display
            disp_mode = _DISP_OFF;
            continue;
            // Do we need this?
            break;
        case _ERR:
            // What do we do here?
            // P sure this does nothing
            disp_mode = _DISP_ERR;
            break;
        case _IDLE:
            // What are we here for?
            temp_cur = get_temp();
            // If the temperature is really high, we probably are blocking the vent
            // Let's turn off to prevent fires and such
            if (temp_cur > 50)
            {
                // We clear error with the watchdog
                // Error will trap us in a loop and starve the watchdog
                // We will then reset back to our regular state
                // Clock may be incorrect!
                ee_save_state();
                error(1);
                continue;
            }
            // We need to take care of temp things if we're this far
            // This chunk takes care of the timer feature
            if (timer)
            {
                // Between when day starts and ends
                if (time / 3600 >= day_hour && time / 3600 < night_hour)
                {
                    // day temp controls
                    if (temp_cur > (day_temp + TEMP_WINDOW))
                    {
                        // Too hot
                        // Use the fan if in fan or Hybrid modes
                        if (climate_control & _COOL)
                        {
                            heat_cur = _FAN;
                        }
                    }
                    else if (temp_cur <= (day_temp - TEMP_WINDOW))
                    {
                        // Too low
                        // Run heater if not in fan mode
                        if (climate_control & _HEAT)
                        {
                            // Heat mode selects between heat1 or heat_both
                            heat_cur = heat_mode;
                        }
                    }
                } // Use else because broken over midnight
                else
                {
                    // day temp controls
                    if (temp_cur > (night_temp + TEMP_WINDOW))
                    {
                        // Too hot
                        if (climate_control & _COOL)
                        {
                            heat_cur = _FAN;
                        }
                    }
                    else if (temp_cur < (night_temp - TEMP_WINDOW))
                    {
                        // Too low
                        if (climate_control & _HEAT)
                        {
                            heat_cur = heat_mode;
                        }
                    }
                }
            }
                // Not timer
            else
            {
                // Handle non-timer temp
                if (temp_cur > (temp_want + TEMP_WINDOW))
                {
                    // Too hot
                    if (climate_control & _COOL)
                    {
                        heat_cur = _FAN;
                    }
                }
                else if (temp_cur < (temp_want - TEMP_WINDOW))
                {
                    // Too low
                    if (climate_control & _HEAT)
                    {
                        heat_cur = heat_mode;
                    }
                }
            }
            // Use state of 'heat' variable to set outputs

            // If temp low && not fan mode, heat on
            // If heat on or fan mode on, feed fan countdown
            switch (heat_cur)
            {
            case _HEAT_ONE:
                // Randomly select a heating element to use so they are used equally
                if (time % 2) HEAT1 = 1, HEAT2 = 0;
                else HEAT1 = 0, HEAT2 = 1;
                fan_timeout = FAN_FEED_HEAT;
                break;
            case _HEAT_BOTH:
                HEAT1 = 1;
                HEAT2 = 1;
                fan_timeout = FAN_FEED_HEAT;
                break;
            case _FAN:
                fan_timeout = FAN_FEED_NO_HEAT;
            }
            // Run fan if not timed out
            if (fan_timeout) FAN = 1;
            else FAN = 0;
            break;
            
            
            // Let's set the timer here
        case _TIMER_SET:
            
            break;
            // And let's set the time
        case _TIME_SET:
            break;
        }
    }
    return;
}

void setup_io();
void setup_therm();
void setup_timer0();
void setup_timer1();
void setup_timer2();
void setup_eeprom();
void setup_watchdog();
void setup_CCP_T3();

void setup()
{
    // Set up OSC
    // Internal OSC
    SCS0 = 1;
    SCS1 = 1;
    INTSRC = 1;
    // 8MHz
    // Would to 2MHz, but the FOSC/4 is already done for us almost everywhere
    IRCF2 = 1;
    IRCF1 = 1;
    IRCF0 = 1;

    // Need to set up analog first to clear up digital pins
    setup_therm();
    // Set up the IO stuff
    setup_io();

    // Blink Timer
    setup_timer0();
    // RTC Timer
    setup_timer1();
    // Scan Timer
    setup_timer2();
    // Set up CCP
    setup_CCP_T3();

    // Set up EEPROM
    setup_eeprom();

    // Enable interrupts globally
    IPEN = 1;
    GIEH = 1;
    GIEL = 1;

    // Set up the watchdog last so it's not starved too early
    setup_watchdog();
}

void setup_io()
{
    // Enable weak pullups
    nRBPU = 0;
    // All these cathodes need to be outputs
    DISP_1_TRIS = 0;
    DISP_1 = 1;
    DISP_2_TRIS = 0;
    DISP_2 = 1;
    LED_CATHODE_TRIS = 0;
    LED_CATHODE = 1;
    BUZZER_TRIS = 0;
    BUZZER = BUZZ_OFF;

    HEAT1_TRIS = 0;
    HEAT2_TRIS = 0;
    FAN_TRIS = 0;
}

void setup_therm()
{
    // Set Up Analog
    // Set TRIS
    THERM_TRIS = 1;
    // Turn on and set up interrupt
    ADON = 1;
    ADIE = 0;
    ADIP = 0;
    ADIF = 0;
    // Select Channel 0
    CHS0 = CHS1 = CHS2 = CHS3 = 0;
    // Set all but AN0 to digital
    PCFG0 = 0;
    PCFG1 = PCFG2 = PCFG3 = 1;
    // Set timer to middle values...
    ACQT2 = 1;
    ACQT1 = ACQT0 = 0;
    // Table says it should be 0b101
    ADCS2 = 1;
    ADCS1 = 0;
    ADCS0 = 1;
}

// Blink/Buzz timer

void setup_timer0()
{
    // 7812.5k

    // Stop it
    TMR0ON = 0;
    // Reset to 0
    TMR0 = 0;
    // 16 bit
    T08BIT = 0;
    // Internal clock
    T0CS = 1;
    // Low priority interrupt
    T0IF = 0;
    T0IP = 0;
    T0IE = 1;
    // Prescaler assigned to this
    PSA = 0;
    // Prescaler = 2^(T0PS2:T0PS1:T0PS0 + 1)
    // 1:256 (7812.5k)
    T0PS0 = 1;
    T0PS1 = 1;
    T0PS2 = 1;

}
//RTC timer

void setup_timer1()
{
    // Stop it
    TMR1ON = 0;
    // 16-bit mode
    T1RD16 = 1;
    // Internal clock
    T1RUN = 0;
    TMR1CS = 0;
    // Disable low Interrupt
    TMR1IF = 0;
    TMR1IP = 1;
    TMR1IE = 1;
    // Prescale 1:8 (freq = FOSC/4/8 = 15.625kHz)
    // If we want to have seconds, do N counts N Hz
    // Use math so we don't have to worry as much in the future
    T1CKPS0 = T1CKPS1 = 1;
    // Set initial value
    unsigned short tmp = (_XTAL_FREQ / 4UL / 8UL);
    TMR1 = 0xFFFFU - tmp;
    // Start!
    TMR1ON = 1;
}

// Scan timer

void setup_timer2()
{
    // Run it at 61 Hz

    // This timer resets when PR2 == TMR2
    TMR2ON = 0;
    // Set prescale to 1:1
    T2CKPS0 = 0;
    T2CKPS1 = 0;
    // Start at 0
    TMR2 = 0;
    // Number TMR2 counts up to and resets at
    // 0x2005 = 2MHz/244Hz.  244Hz = 61Hz * 4 (Need to be four times as fast so 
    //      one of them seems to have a scan rate of 60Hz)
    PR2 = 0x2005;
    // 1:1 postscale
    T2OUTPS0 = 0;
    T2OUTPS1 = 0;
    T2OUTPS2 = 0;
    T2OUTPS3 = 0;
    // Enable Low Interrupt
    TMR2IF = 0;
    TMR2IP = 0;
    TMR2IE = 1;
    // Start!
    TMR2ON = 1;
}

void setup_eeprom()
{
    // Read/write eeprom, not flash
    EEPGD = 0;
    CFGS = 0;
    // Don't erase and write, just write
    FREE = 0;
    // Clear error flag
    WRERR = 0;
    // Disable writing to prevent spurious writes
    WREN = 0;
    // Enable low interrupt
    EEIF = 0;
    EEIP = 0;
    EEIE = 1;
}

// So in case of deadlocks we can restart
// Exploited in the error routine

void setup_watchdog()
{
    // Prescaler set in #pragma configs
    // Enable Watchdog
    SWDTEN = 1;
    // Feed it
    asm("clrwdt");
}

// IR Timer & CCP

void setup_CCP_T3()
{
    // 2MHz

    // Disable so we can set everything first.  Probably superfluous
    TMR3ON = 0;
    // Internal clock, Fosc/4
    TMR3CS = 0;
    // Set to 16-bit mode
    T3RD16 = 1;
    // Assign T3 to both CCP modules
    T3CCP2 = 1;
    T3CCP1 = 1;
    // 1:1 prescale
    T3CKPS0 = 0;
    T3CKPS1 = 0;

    // Enable high interrupts for T3
    // IR is very timing dependent so don't wanna get stuck waiting
    TMR3IE = 1;
    TMR3IF = 0;
    TMR3IP = 0;

    // Config frequency & clock source
    // CCP Prescaler 1:1
    CCP1M0 = 0; // Rising edge is 0, falling 1
    CCP1M1 = 0; // If = 1, prescale the CCP ()
    CCP1M2 = 1; // Set to use CCP
    CCP1M3 = 0; // If = 1, Compare or PWM

    // Low priority Interrupt
    CCP1IP = 0;
    CCP1IF = 0;
    CCP1IE = 1;

    TMR3ON = 1;

}

// Errors will go into a loop and then device will reset with watchdog

void error(char code)
{
    // Let's make this mode not interfere with the display
    disp_mode = _DISP_ERR;
    // 'E' -> segments ADEFG
    disp[0] = num_to_disp('E') | BIT7; // Don't blink
    disp[1] = num_to_disp(code) | BIT7;
    set_key_led_disp_tris(0);

    // If it's an error, we shouldn't be doing things...
    HEAT1 = 0;
    HEAT2 = 0;
    FAN = 0;
    BUZZER = BUZZ_OFF;
    int i;
    while (1)
    {
        DISP_1 = 0;
        DISP_2 = 1;
        LED_CATHODE = 1;
        set_digit(disp[0] ^ (BLINK * BIT7));
        // Wait a bit (should be a little under a millisecond or two)
        i = 100;
        while (--i);

        DISP_1 = 1;
        DISP_2 = 0;
        LED_CATHODE = 1;
        set_digit(disp[1] ^ (BLINK * BIT7));
        // Wait a bit
        i = 100;
        while (--i);

        DISP_1 = 1;
        DISP_2 = 1;
        LED_CATHODE = 0;
        // Set a combination of LEDs that would never exist in the wild
        set_LEDs(LED_C_MASK | LED_F_MASK);
        // Wait a bit
        i = 1000;
        while (--i);
    }
}

float get_temp()
{
    // Start ADC
    GO = 1;
    while (GO);
    // Cast the two ten bit numbers to doubles 
    // Calculate the voltage on the pin
    double voltage = ((double) ADRES) * 5.0 / ((double) (1L << 16));
#ifdef RTD
    // Since we're dealing with an RTD, there needs to be some mathematical magic
    // to make it linear.  I think... http://ww1.microchip.com/downloads/en/AppNotes/00687b.pdf
    // https://en.wikipedia.org/wiki/Resistance_thermometer
    double resistance = (voltage / (5 - voltage)) * TEMP_R_DIV;
    // Quadratic formula
    double temp = sqrt(TEMP_A * TEMP_A - 4 * TEMP_B);
    temp = (-TEMP_A + temp) / (2 * TEMP_B);
#endif
#ifdef THERMISTER
    // RTD equation didn't work...
    // Try thermister equation?
    double resistance = (voltage / (5 - voltage)) * TEMP_R_DIV;
    // 1/T = a + b*ln(R) + c*(ln(R))^3
    double temp = (1 / (TEMP_A + TEMP_B * log(resistance) + TEMP_C * log(resistance)));
#endif
    return temp;
}

char sample_inputs()
{
    // Set to inputs
    set_key_led_disp_tris(1);
    unsigned char ret = 0;
    // Button pressed = 0, not pressed = 1
    // 1*0b1000 = 0b1000
    // 0*0b1000 = 0b0000
    // Multiplication makes it so we can mask a bit that is contingient on another
    // Bit being set without needing ternary operators
    ret |= KEY1 * KEY1_MASK;
    ret |= KEY2 * KEY2_MASK;
    ret |= KEY3 * KEY3_MASK;
    ret |= KEY4 * KEY4_MASK;
    ret |= KEY5 * KEY5_MASK;
    ret |= KEY6 * KEY6_MASK;

    // invert to make 0 = off, 1 = on logic
    return ~ret;
}

void set_LEDs(unsigned char mask)
{
    set_key_led_disp_tris(0);
    LED_CATHODE = 0;
    // Since the masks are powers of two, this basically just does bit shifts
    // The lowest bit will be 0 or one depending on what 'mask' is at the bit
    LED1 = mask / LED1_MASK;
    LED2 = mask / LED2_MASK;
    LED3 = mask / LED3_MASK;
    LED4 = mask / LED4_MASK;
    LED5 = mask / LED5_MASK;
    LED_C = mask / LED_C_MASK;
    LED_F = mask / LED_F_MASK;
}

void buzzer(char duration)
{
    // Buzz for duration number of TMR0 cycles
    buzz = duration;
    // Set the output to on
    BUZZER = buzz > 0 ? BUZZ_ON : BUZZ_OFF;
}

void set_key_led_disp_tris(unsigned char dir)
{
    // Use LEDs because there are one more than there are keys
    // Could have used display segments, but I chose LEDs just cause
    LED1_TRIS = dir; // This is RB1.  I can't be an input.  Will it die if we assign anything to it?
    LED2_TRIS = dir;
    LED3_TRIS = dir;
    LED4_TRIS = dir;
    LED5_TRIS = dir;
    LED_C_TRIS = dir;
    LED_F_TRIS = dir;
}

void ee_write_byte(unsigned char addr, unsigned char data)
{
    // Enable writes
    WREN = 1;
    EEADR = addr;
    EEDATA = data;
    // Disable interrupts so it doesn't break...
    // Man said to
    // This thing avoids spurious writes
    GIE = 0;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    GIE = 1;
    // Do the write
    WR = 1;
    // Interrupt will clean up
}

unsigned char ee_read_byte(unsigned char addr)
{
    EEADR = addr;
    RD = 1;
    // Accessible next cycle
    return EEDATA;
}

void ee_save_state()
{
    ee_write_byte(EE_TIME_ADDR, time);
    ee_write_byte(EE_WANT_TEMP_ADDR, temp_want);
    ee_write_byte(EE_DAY_TIME_ADDR, day_hour);
    ee_write_byte(EE_DAY_TEMP_ADDR, day_temp);
    ee_write_byte(EE_NIGHT_TIME_ADDR, night_hour);
    ee_write_byte(EE_NIGHT_TEMP_ADDR, night_temp);
    ee_write_byte(EE_STATE_ADDR, climate_control | heat_mode << 4);
}

void ee_read_state()
{
    time = ee_read_byte(EE_TIME_ADDR);
    temp_want = ee_read_byte(EE_WANT_TEMP_ADDR);
    temp_want = temp_want ? temp_want : 20;
    day_hour = ee_read_byte(EE_DAY_TIME_ADDR);
    day_temp = ee_read_byte(EE_DAY_TEMP_ADDR);
    day_temp = day_temp ? day_temp : 20;
    night_hour = ee_read_byte(EE_NIGHT_TIME_ADDR);
    night_temp = ee_read_byte(EE_NIGHT_TEMP_ADDR);
    night_temp = night_temp ? night_temp : 20;
    unsigned char mode = ee_read_byte(EE_STATE_ADDR);
    climate_control = 0x0F & mode;
    climate_control = (climate_control ? climate_control : (_HEAT | _COOL));
    heat_mode = 0x0F & (mode >> 4);
    heat_mode = (heat_mode != _NONE ? heat_mode : _HEAT_BOTH);
}

// Do we need this?
// The datasheet says that we don't need it if we are storing
// infrequently changing variables, but then it says
// we need to do this if we don't use frequently changing data

void ee_refresh()
{
    WREN = 1;
    EEADR = sizeof (eeprom_addrs);
    while (--EEADR)
    {
        RD = 1;
        GIE = 0;
        EECON2 = 0x55;
        EECON2 = 0xAA;
        WR = 1;
        while (WR);
        GIE = 0;
    }
    WREN = 0;
}

// Don't really know what the difference is between high and low
// but the clock is most important

void interrupt high_priority ISR_high()
{
    if (TMR1IE && TMR1IF)
    {
        // count to 15625 (Hz)
        // Put at top to reduce call overhead before resetting.
        // Compiler is smart enough to make this a constant during compile time
        // Also, gonna add it in case timer rolled over and started again and
        // already stated counting.  This is actually a good thing and we want
        // take advantage of it
        unsigned short tmp = (_XTAL_FREQ / 32UL);
        TMR1 += 0xFFFFU - tmp;
        // Clear flag
        TMR1IF = 0;
        // Advance seconds and roll over at 24 hours
        time = (long) (time + 1) % (3600L * 24L);
        // Count down fan counter
        fan_timeout--;
        // Save time to eeprom
        ee_write_byte(time, EE_TIME_ADDR);
        // Don't process any more interrupts in this interrupt call
        return;
    }
}

// Blinking and scanning are less important

void interrupt low_priority ISR_low()
{
    // Timer for blinking
    // Runs at 2MHz/256 = 7812.5k
    // To make the blink value change ever 1/2 second, reset it after 3906 pulses
    if (TMR0IE && TMR0IF)
    {
        TMR0IF = 0;
        // We only want to run it for 3906 pulses = ~4Hz
        TMR0 = 0xFFFF - 0xF42;

        // Count up to 4, and then each time we ask for blink, we should divide
        // it by 2.  If blink is less than 2, it will be zero, and more than
        // 2 it will be one.  It will never be 4 so it will never be two
        blink = (blink + 1) % (BLINK_DIV * 2);

        // Decrement so we can use a duration
        // Decrement before so we don't deal with a negative number
        // Also do the "&&" so that we don't decrement forever and it only
        // decrements every time it's not zero
        if (buzz && --buzz)
        {
            // Turn buzzer on
            BUZZER = BUZZ_ON;
        } // turn off if buzz = 0
        else
        {
            BUZZER = BUZZ_OFF;
        }
        // Don't process any more interrupts in this interrupt call
        return;
    }
    if (TMR2IE && TMR2IF)
    {
        static scan_state cur = _KEYS;
        unsigned char ins = 0;
        static unsigned char ins_prev = 0;
        unsigned char mask_leds;
        static unsigned char long_press;
        // Use this variable to make buttons cause repeating actions.
        // Mostly thinking of up/down
        // Since only one button can be pressed once, then we only need one of these
        static unsigned char button_repeat = 0;
        // Use this so we can have half duty cycles and such
        static unsigned char counter = 0;
        static timer_set_state timer_are_setting;

        TMR2IF = 0;
        // Turn off all things before processing
        LED_CATHODE = 1;
        DISP_1 = 1;
        DISP_2 = 1;
        // Advance variable through 4 modes
        cur = (cur + 1) % 4;
        switch (cur)
        {
        case _KEYS:
            // Keep track of old states to catch edges
            ins_prev = ins;
            // Get new input
            ins = sample_inputs();

            buzzer(BUZZ_SHORT);
            unsigned char i = BIT7;
            // if not a single bit, the log will not be an integer
            if (log(ins) / log(2) != (int) (log(ins) / log(2)))
            {
                // Just pretend no buttons are pressed
                ins = 0;
            }
            // If any button is pressed, let's increment the long press timer
            if (ins) long_press++;
            // IF none are pressed (or multiple) reset the timer
            if (!ins) long_press = 0;
            do
            {
                // Very confusing looking, but it masks the button bit,
                // the inversion of the previous button bit, and the button mask
                // Basically goes to its case when button is a rising edge
                switch (ins & ~ins_prev & i)
                {
                case KEY1_MASK: //Timer
                    // Inverts bottom bit
                    if (state == _IDLE)
                    {
                    timer = 1^timer;
                    }
                    else if (state == _TIMER_SET)
                    {
                      
                    }
                    break;
                case KEY2_MASK: //Inc
                    (*cur_number)++;
                    break;
                case KEY3_MASK: //Dec
                    (*cur_number)--;
                    break;
                case KEY4_MASK: //Mode
                    if (state != _TIME_SET)
                    {
                        unit = 1^unit;
                    }
                    else
                    {
                        state = _IDLE;
                        ee_write_byte(time, EE_TIME_ADDR);
                    }
                    break;
                case KEY5_MASK: //Power
                    buzzer(BUZZ_LONG);
                    // If we are off, then we should clear stuff and then we don't need to check other buttons
                    if (state == _OFF)
                    {
                        state = _IDLE;
                        break;
                    }
                    else // Any state other than off, we should turn off
                    {
                        state = _OFF;
                    }
                    break;
                case KEY6_MASK: //Eco
                    if (heat_mode == _HEAT_BOTH)
                    {
                        heat_mode = _HEAT_ONE;
                    }
                    else if (heat_mode == _HEAT_ONE)
                    {
                        heat_mode = _HEAT_BOTH;
                    }
                    break;
                }
                if (long_press > LONG_PRESS)
                {
                    // Decrement so very long button presses don't roll over
                    long_press--;
                    // Held on
                    // Use for long button presses
                    switch (ins & ins_prev & i)
                    {
                    case KEY1_MASK: //Timer
                        // Set timer
                        state = _TIMER_SET;
                        break;
                    case KEY2_MASK: //Inc
                        // Repeatedly increase number
                        // use blink val?
                        break;
                    case KEY3_MASK: //Dec
                        // Repeatedly decrease number
                        break;
                    case KEY4_MASK: //Mode
                        // Set time
                        state = _TIME_SET;
                        break;
                    case KEY5_MASK: //Power
                        break;
                    case KEY6_MASK: //Eco

                        break;
                    }
                }
                // Falling edge
                // Use to reset long button timers
                switch (~ins & ins_prev & i)
                {
                case KEY1_MASK: //Timer
                    break;
                case KEY2_MASK: //Inc
                    break;
                case KEY3_MASK: //Dec
                    break;
                case KEY4_MASK: //Mode
                    break;
                case KEY5_MASK: //Power
                    break;
                case KEY6_MASK: //Eco
                    break;
                }
            }
            while (i = i << 1);
            break;
        case _LEDS:
            switch (state)
            {
            case _OFF:
                // Only illum power LED when off
                mask_leds = LED5_MASK;
                break;
            case _IDLE:
                mask_leds =
                        // Timer
                        (timer_led ? LED1_MASK : 0) |
                        // Day
                        (timer && (((time / 3600) >= day_hour && (time / 3600) < night_hour) ? LED2_MASK : 0)) |
                        // Night
                        (timer && (((time / 3600) < day_hour || (time / 3600) >= night_hour) ? LED3_MASK : 0)) |
                        // If both heats, light eco at 100%, else if one, at 50%.  If only cool, turn off
                        (heat_mode == _HEAT_BOTH ? LED4_MASK : ((heat_mode == _HEAT_ONE && (counter++ & 1)) ? LED4_MASK : 0)) |
                        // Power
                        (power_led ? LED5_MASK : 0) |
                        // Celsius or Fahrenheit
                        (unit ? LED_F_MASK : LED_C_MASK);
                break;
            case _TIMER_SET:
                mask_leds =
                        // Blink timer LED
                        BLINK_DIV ? LED1_MASK : 0 |
                        // Power LED
                        (power_led ? LED5_MASK : 0);
                break;
            case _TIME_SET:
                mask_leds =
                        // Blink timer LED
                        BLINK_DIV ? LED1_MASK : 0 |
                        // Blink Eco because it's above the mode button
                        BLINK_DIV ? LED4_MASK : 0 |
                        // Power LED
                        (power_led ? LED5_MASK : 0);

            }
            set_LEDs(mask_leds);
            break;
        case _DISP1:
            DISP_1 = 0;
            DISP_2 = 1;
            switch (disp_mode)
            {
                // Turn off the display
                // No need to save its contents
            case _DISP_OFF:
                disp[0] = 0;
                break;
                // Don't change the display
            case _DISP_ERR:
                break;
//            case _DISP_TEMP_CUR:
//                disp[0] = temp_cur / 10;
//                break;
            case _DISP_TEMP_WANT:
                disp[0] = temp_want / 10;
                break;
            case _DISP_TIMER:
                switch (timer_are_setting)
                {
                case _HIGH_HOUR:
                case _HIGH_TEMP:
                case _LOW_HOUR:
                case _LOW_TEMP:
                }
                break;
            case _DISP_TIME:
                break;
            }
            set_digit(disp[0]);
            break;
        case _DISP2:
            DISP_1 = 1;
            DISP_2 = 0;
            set_digit(disp[1]);
            break;
        }
        // Don't process any more interrupts in this interrupt call
        return;
    }
    if (EEIE && EEIF)
    {
        EEIF = 0;
        // Disable more writes until we write again
        WREN = 0;
        // Don't process any more interrupts in this call
        return;
    }
    if (CCP1IE && CCP1IF)
    {
        // Make sure timer is running if we get an edge.
        TMR3ON = 1;
        // Do a short because maxes at 65535Hz, which is plenty high.
        // also do a sanity check on the resultant freq from detection 
        //   to make sure it is a sane value, e.g. between 32kHz and 44kHz?
        static unsigned short freq = 0;
#if 0
        if (!freq)
        {

        }
        switch (timer_val)
        {
        case short -offset ... short +offset - 1:
            data = same as last;
            // Reset timer
            break;
        case short +offset .. long -offset - 1:
            // Same
            // Not sure?;
            break;
        case long -offset ... long +offset - 1:
            // Opposite of last bit
            break;
        default:
            // What to do? Do we throw it out?  Discard the sample? Exit out?
        }
#endif
#if 0
        // Reset timer for next edge
        if (!pulse_num)
        {
            times[pulse_num] = CCPR1;
        }
        // Change edge detect
        CCP1M0 = !CCP1M0;
        if ((TMR3 - times[pulse_num]) > IR_LONG_PULSE + IR_VARIABILITY)
        {
            TMR3ON = 0;
            TMR3 = 0;
        }
        pulse_num++;
#endif
        // Don't process any more interrupts in this call
        return;
    }
    // If it overflowed, let's just stop the timer until we hit another pulse
    if (TMR3IE && TMR3IF)
    {
        TMR3IF = 0;
        TMR3ON = 0;
        return;
    }
}