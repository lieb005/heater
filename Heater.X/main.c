/*
 * File:   main.c
 * Author: talia
 *
 * Created on October 9, 2016, 3:47 PM
 */


#include <xc.h>
#include "main.h"
#include "rc5.h"
#include "disp.h"

#pragma config FOSC = INTOSCIO_EC
#pragma config MCLRE = 1
#pragma config PBADEN = 1
#pragma config WDT = 0
#pragma config PWRT = 1
#pragma config LPT1OSC = 0
#pragma config DEBUG = 1

#define _FOSC 500000

void setup();
void set_LEDs(char mask);
char sample_inputs();
float get_temp();
// Save state to eeprom
void save_state();
// Read state from eeprom
void read_state();
// To call if overheat or any other number of reasons
void error(char code);
unsigned char num_to_disp(char num);
void buzzer(char duration);

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
// Is the timer on?  Are we in *C
bit timer = 0, unit = 0;
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
// Current Temperature
float temp_cur;
// Variable of what's on the display.  BIT7 is the on/off bit
unsigned char disp[2];
// Both countdown variables to control blink/buzz diration
unsigned char buzz = 0, blink = 0;

bit timer_led = 0, power_led = 0;
// Use like a countdown, but when we want fan on, feed this like a watchdog
// Lets the fan blow out all the hot air if the heat turns off
char fan_timeout = 0;

void main(void)
{
	setup();
	while (1)
	{
		if (state == _ERR)
		{
			HEAT1 = 0;
			HEAT2 = 0;
			FAN = 0;
			
			// How do we clear error?
			// Overheat is currently the only error
		}
		temp_cur = get_temp();
		if (temp_cur > 40)
		{
			state = _ERR;
			error(1);
			continue;
		}
		if (state != _OFF) continue;
		if (timer)
		{
			// Between when day starts and ends
			if (time / 3600 >= day_hour && time / 3600 < night_hour)
			{
				// day temp controls
				if (temp_cur > (day_temp + 3))
				{
					// Too hot
					// Use the fan if in fan or Hybrid modes
					if (climate_control != _HEAT)
					{
						heat_cur = _FAN;
					}
				}
				else if (temp_cur <= (day_temp + 3))
				{
					// Too low
					// Run heater if not in fan mode
					if (climate_control != _FAN)
					{
						// Heat mode selects between heat1 or heat_both
						heat_cur = heat_mode;
					}
				}
			}
				// Use else because broken over midnight
			else
			{
				// day temp controls
				if (temp_cur > (night_temp + 3))
				{
					// Too hot
					if (climate_control != _HEAT)
					{
						heat_cur = _FAN;
					}
				}
				else if (temp_cur < (night_temp + 3))
				{
					// Too low
					if (climate_control != _FAN)
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
			if (temp_cur > (temp_want + 3))
			{
				// Too hot
				if (climate_control != _HEAT)
				{
					heat_cur = _FAN;
				}
			}
			else if (temp_cur < (temp_want + 3))
			{
				// Too low
				if (climate_control != _FAN)
				{
					heat_cur = heat_mode;
				}
			}
		}
		// Use state of heat variable to set outputs

		// If temp low && not fan mode, heat on
		// If heat on or fan mode on, feed fan countdown
		switch (heat_cur)
		{
		case _HEAT1:
			// Randomly select a heating element to use so they are used equally
			if (time%2) HEAT1 = 1, HEAT2 = 0;
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
	}
	return;
}

void setup_io();
void setup_therm();
void setup_timer0();
void setup_timer1();
void setup_timer2();

void setup()
{
	// Set up OSC
	// Internal OSC
	SCS0 = 1;
	SCS1 = 1;
	INTSRC = 1;
	// 500kHz
	// Would to 125kHz, but the FOSC/4 is already done for us almost everywhere
	IRCF2 = 0;
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

	// Enable interrupts globally
	IPEN = 1;
	GIEH = 1;
	GIEL = 0;
}

void setup_io()
{
	// Enable weak pullups
	nRBPU = 0;
	// All these cathodes need to be outputs
	DISP_1_TRIS = 0;
	DISP_1_TRIS = 1;
	DISP_2_TRIS = 0;
	DISP_2_TRIS = 1;
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
	// Stop it
	TMR0ON = 0;
	// Reset to 0
	TMR0 = 0;
	// 16 bit
	T08BIT = 0;
	// Internal clock
	T0CS = 1;
	// Low priority interrupt
	T0IE = 1;
	T0IF = 0;
	T0IP = 0;
	// Prescaler assigned to this, not WDT
	PSA = 0;
	// 1:256 (125kHz / 256 =~ 488.3Hz)
	T0PS0 = T0PS1 = T0PS2 = 1;

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
	TMR1IE = 1;
	TMR1IF = 0;
	TMR1IP = 1;
	// Prescale 1:8 (freq = FOSC/4/8 = 15.625kHz)
	// If we want to have seconds, do N counts N Hz
	// Use math so we don't have to worry as much in the future
	T1CKPS0 = T1CKPS1 = 1;
	// Set initial value
	TMR1 = 0xFFFF - (int) ((double) _FOSC / 4.0 / 8.0);
	// Start!
	TMR1ON = 1;
}

// Scan timer

void setup_timer2()
{
	// This timer resets when PR2 == TMR2
	TMR2ON = 0;
	// Set prescale to 1:2 (125k/2 = 62.5k)
	T2CKPS0 = 1;
	T2CKPS1 = 0;
	// Start at 0
	TMR2 = 0;
	// Number TMR2 counts up to and resets at
	// 256 = 62.5k/244.  244 = 61Hz * 4
	PR2 = 0xFF;
	// 1:2 postscale because 
	T2OUTPS0 = 0;
	T2OUTPS1 = 0;
	T2OUTPS2 = 0;
	T2OUTPS3 = 0;
	// Enable Low Interrupt
	TMR2IE = 1;
	TMR2IF = 0;
	TMR2IP = 0;
	// Start!
	TMR2ON = 1;
}

void error(char code)
{
	// E -- ADEFG
	disp[0] = num_to_disp('E');
	disp[1] = num_to_disp(code);
	set_digit(disp[0], 0);
	set_digit(disp[1], 1);
}

float get_temp()
{
	// Start ADC
	GO = 1;
	while (GO);
	// Cast the two ten bit numbers to doubles 
	float temp = ((double) ADRES) * 5.0 * TEMP_K/ ((double) (1L << 16)) + TEMP_OFFSET;
	// Need to do something like temp = temp * K + offset;
	return temp;
}

char sample_inputs()
{
	// Set to inputs
	set_key_led_disp_tris(1);
	// Pull cathodes high
	LED_CATHODE = 1;
	DISP_1 = 1;
	DISP_2 = 1;
	// Set to inputs
	TRISB = 0xFF;
	return PORTB;
}

void set_LEDs(char mask)
{
	TRISB = 0;
	DISP_1 = 1;
	DISP_2 = 1;
	LED_CATHODE = 0;
	LATB = mask;
}

void buzzer(char duration)
{
	// Buzz for duration number of TMR0 cycles
	buzz = duration;
	// Set the output to on
	BUZZER = BUZZ_ON;
}

void set_key_led_disp_tris(unsigned char dir)
{
	// Use LEDs because there are one more than there are keys
	// Could have used display segments, but I chose LEDs just cause
	LED1_TRIS = dir; // This is RB1.  I can't be an input.  Will it die?
	LED2_TRIS = dir;
	LED3_TRIS = dir;
	LED4_TRIS = dir;
	LED5_TRIS = dir;
	LED_C_TRIS = dir;
	LED_F_TRIS = dir;
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
		TMR1 += 0xFFFF - (_FOSC / 32);
		// Clear flag
		TMR1IF = 0;
		// Advance seconds and roll over at 24 hours
		time = (long) (time + 1) % (3600L * 24L);
		// Count down fan counter
		fan_timeout--;
		// Don't process any more interrupts in this interrupt call
		return;
	}
}

// Blinking and scanning are less important

void interrupt low_priority ISR_low()
{
	// Blink/buzz timer
	if (TMR0IE && TMR0IF)
	{
		blink = ~blink;
		// Decrement so we can use a duration
		// Decrement before so we don't deal with a negative number
		// Also do the "&&" so that we don't decrement forever and it only
		// decrements every time it's not zero
		if (buzz && --buzz)
		{
			// Turn buzzer on
			BUZZER = BUZZ_ON;
		}
		// turn off if buzz = 0
		else
		{
			BUZZER = BUZZ_OFF;
		}
		// Don't process any more interrupts in this interrupt call

		return;
	}

	scan_state cur = _KEYS;
	unsigned char ins = 0;
	static unsigned char ins_prev = 0;
	unsigned char mask_leds;

	if (TMR2IE && TMR2IF)
	{
		TMR2IF = 0;
		switch (cur)
		{
		case _KEYS:
			// Keep track of old states to catch edges
			ins_prev = ins;
			// Get new input
			ins = sample_inputs();
			// We really only care about rising edges...
			// Power
			if ((ins & KEY5_MASK) && !(ins_prev && KEY5_MASK))
			{
				state = ((state != _OFF) ? _OFF : _IDLE);
			}
			// If we are off, then we should clear stuff and then we don't need to check other buttons
			if (state == _OFF)
			{
				disp[0] = 0;
				disp[1] = 1;
				HEAT1 = 0;
				HEAT2 = 0;
				break;
			}
			// Timer button
			if ((ins & KEY1_MASK) && !(ins_prev && KEY1_MASK))
			{
				//key1_press_time = time;
			}
			//if ((ins & KEY1_MASK) && )
			// Dec
			if ((ins & KEY2_MASK) && !(ins_prev && KEY2_MASK))
			{
				(*cur_number)++;
			}
			//if ((ins & KEY2_MASK) && )
			// Inc
			if ((ins & KEY3_MASK) && !(ins_prev && KEY3_MASK))
			{
				(*cur_number)--;
			}
			// Mode
			if ((ins & KEY4_MASK) && !(ins_prev && KEY4_MASK))
			{
				unit = 1 - unit;
			}
			// Eco
			if ((ins & KEY6_MASK) && !(ins_prev && KEY6_MASK))
				buzzer(1);
			break;
		case _LEDS:
			DISP_1 = 1;
			DISP_2 = 1;
			mask_leds = (timer_led ? LED1_MASK : 0) |
					(timer && (((time / 3600) >= day_hour && (time / 3600) < night_hour) ? LED2_MASK : 0)) |
					(timer && (((time / 3600) < day_hour || (time / 3600) >= night_hour) ? LED3_MASK : 0)) |
					((heat_mode != HEAT1 && heat_mode != HEAT2) ? LED4_MASK : 0) |
					(power_led ? LED5_MASK : 0) |
					(unit ? LED_F_MASK : LED_C_MASK);
			set_LEDs(mask_leds);
			break;
		case _DISP1:
			LED_CATHODE = 1;
			set_digit(disp[0], 0);
			break;
		case _DISP2:
			LED_CATHODE = 1;
			set_digit(disp[1], 1);
			break;
		}
		// Don't process any more interrupts in this interrupt call
		return;
	}
}