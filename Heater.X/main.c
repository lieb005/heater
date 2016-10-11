/*
 * File:   main.c
 * Author: talia
 *
 * Created on October 9, 2016, 3:47 PM
 */


#include <xc.h>
#include "main.h"
#include "rc5.h"

#pragma config FOSC = INTOSCIO_EC
#pragma config MCLRE = 1
#pragma config PBADEN = 1
#pragma config WDT = 0
#pragma config PWRT = 1
#pragma config LPT1OSC = 0
#pragma config DEBUG = 0

#define _FOSC 125000

void setup();
void set_LEDs(char mask);
void set_digit(unsigned char dig, char num);
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

char temp_want;
// Numbur of seconds since midnight
long time;
char high_hour, low_hour;
char high_temp, low_temp;
bit timer = 0, unit = 0;
char *cur_number = &temp_want;
global_state state = OFF;
heat_state heat_mode;
float temp_cur;
unsigned char disp[2];

bit timer_led = 0, power_led  = 0;

void main(void)
{
	setup();
	while (1)
	{
		if (state == ERR)
		{
			// How do we clear error?
			// Overheat is currently the only error
		}
		temp_cur = get_temp();
		if (temp_cur > 40)
		{
			state = ERR;
			error(1);
		}
		if (state != OFF) continue;
		if (timer)
		{
			// Handle timer temps
		}
		else
		{
			// Handle non-timer temp
		}
		// use state of heat variable to set outputs
	}
	return;
}

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
	// 125kHz
	IRCF2 = 0;
	IRCF1 = 0;
	IRCF0 = 1;

	// Need to set up analog first to clear up digital pins
	setup_therm();

	// Enable weak pullups
	nRBPU = 0;
	// All these cathodes need to be outputs
	DISP_1_TRIS = 0;
	DISP_2_TRIS = 0;
	LED_CATHODE_TRIS = 0;
	BUZZER_TRIS = 0;

	// Set up interrupt basics
	IPEN = 1;
	GIEH = 1;
	GIEL = 0;

	// Blink Timer
	setup_timer0();
	// RTC Timer
	setup_timer1();
	// Scan Timer
	setup_timer2();

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
	// Prescaler
	PSA = 0;
	// 1:256
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
	// Prescale 1:8 (freq = 125kHz/8 = 15.625kHz)
	T1CKPS0 = T1CKPS1 = 1;
	// Set initial value
	TMR1 = 0xFFFF - 15625;
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
	float temp = ((double) ADRES) / ((double) (1L << 16));
	// Need to do something like temp = temp * K + offset;
	return temp;
}

char sample_inputs()
{
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

unsigned char num_to_disp(char num)
{
	unsigned char ret;
	switch (num)
	{
	case 0 ... 9:
		ret = font[num];
		break;
	case 'E':
	case 'e':
		ret = (A | 0 | 0 | D | E | F | G);
		break;
	default:
		ret = (0);
		break;
	}
	return ret;
}

// Digits are 0 or 1
void set_digit(unsigned char dig, char num)
{
	// Output now
	TRISB = 0x00;
	// Don't run LEDs
	LED_CATHODE = 1;
	// Select which digit we show.  They're active low
	DISP_1 = num;
	DISP_2 = !num;
	// Need to do this long hand because DISP_A thru DISP_G aren't necessarily linear
	DISP_A = 0!=(dig & DISP_A_MASK);
	DISP_B = 0!=(dig & DISP_B_MASK);
	DISP_C = 0!=(dig & DISP_C_MASK);
	DISP_D = 0!=(dig & DISP_D_MASK);
	DISP_E = 0!=(dig & DISP_E_MASK);
	DISP_F = 0!=(dig & DISP_F_MASK);
	DISP_G = 0!=(dig & DISP_G_MASK);
}

void buzzer(char duration)
{
	// start timer?
	// set buzzer to high
	// Use interrupt to turn off
}

void interrupt high_priority ISR_high()
{
	if (TMR1IE && TMR1IF)
	{
		TMR1IF = 0;
		// count to 15625 (Hz)
		TMR1 = 0xFFFF - 15625;
		time = (long)(time + 1) % (3600L * 24L);
		return;
	}
	
}

void interrupt low_priority ISR_low()
{
	// Blink/buzz timer
	if (TMR0IE && TMR0IF)
	{
		
	}
	
	scan_state cur = KEYS;
	unsigned char ins = 0;
	static unsigned char ins_prev = 0;
	unsigned char mask_leds;

	if (TMR2IE && TMR2IF)
	{
		TMR2IF = 0;
		switch (cur)
		{
		case KEYS:
			// Keep track of old states to catch edges
			ins_prev = ins;
			// Get new input
			ins = sample_inputs();
			// We really only care about rising edges...
			// Power
			if ((ins & KEY5_MASK) && !(ins_prev && KEY5_MASK))
			{
				state = ((state != OFF) ? OFF : IDLE);
			}
			// If we are off, then we should clear stuff and then we don't need to check other buttons
			if (state == OFF) break;
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
		case LEDS:
			mask_leds = (timer_led ? LED1_MASK : 0) |
					(timer && (((time / 3600) >= high_hour && (time / 3600) < low_hour) ? LED2_MASK : 0)) |
					(timer && (((time / 3600) < high_hour || (time / 3600) >= low_hour) ? LED3_MASK : 0)) |
					((heat_mode != HEAT1 && heat_mode != HEAT2) ? LED4_MASK : 0) |
					(power_led ? LED5_MASK : 0) |
					(unit ? LED_F_MASK : LED_C_MASK);
			set_LEDs(mask_leds);
			break;
		case DISP1:
			set_digit(disp[0], 0);
			break;
		case DISP2:
			set_digit(disp[1], 1);
			break;
		}
	}
}