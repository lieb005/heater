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
#pragma config PBADEN = 0
#pragma config WDT = 0
#pragma config PWRT = 1
#pragma config LPT1OSC = 0
#pragma config DEBUG = 0

#define _FOSC 125000

void setup();
void set_LEDs(char mask);
void set_disp(char num, char digit_mask);
char sample_inputs();
// Save state to eeprom
void save_state();
// Read state from eeprom
void read_state();
// To call if overheat or any other number of reasons
void error(char code);
                                  
char temp_want;
float temp_cur;
char hour_cur, high_hour, low_hour;
char high_temp, low_temp;

void main(void)
{
	setup();
	static bit timer = 0, unit = 0;
	enum heat heat_mode = HEAT2;
	char ins, mask_leds;
	static char *cur_number = &temp_want;
	static char state = OFF;
	digit display[2];
	while (1)
	{
		// Power
		if (ins & KEY5_MASK) state = ((state !=OFF) ? OFF : IDLE);
		if (TMR1) 
		if (state == OFF) continue;
		// Timer
		if (ins & KEY1_MASK) timer;
		// Dec
		if (ins & KEY2_MASK) (*cur_number)++;
		// Inc
		if (ins & KEY3_MASK) (*cur_number)--;
		// Mode
		if (ins & KEY4_MASK) unit = 1 - unit;
		// Eco
		if (ins & KEY6_MASK) ;
		mask_leds = ((timer == 1) ? LED1_MASK : 0) |
					((hour_cur >= high_hour && hour_cur < low_hour) ? LED2_MASK : 0) |
					((hour_cur < high_hour || hour_cur >= low_hour) ? LED3_MASK : 0) |
					(((heat_mode != heat.HEAT1 && heat_mode != heat.HEAT2)) ? LED4_MASK : 0) |
					(LED5_MASK) |
					(unit == 0 ? LED_C_MASK : LED_F_MASK);
		set_LEDs(mask_leds);
		
		set_disp(display);
		temp_cur = get_temp();
		if (temp_cur > 40) error();
		
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
	// Internal osc
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

// Blink timer
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
	// High priority interrupt
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
	TMR1IE = 0;
	TMR1IF = 0;
	TMR1IP = 0;
	// Prescale 1:8
	T1CKPS0 = T1CKPS1 = 1;
}

// Scan timer
void setup_timer2()
{
	// This timer resets when PR2 == TMR2
	TMR2ON = 0;
	// No out prescale
	T2OUTPS0 = T2OUTPS1 = T2OUTPS2 = T2OUTPS3 = 0;
	// Set prescale to 1:16
	T2CKPS0 = T2CKPS1 = 1;
	// Start at 0
	TMR2 = 0;
	// Number TMR2 counts up to and resets at
	// 208 * 3 = 625.  625 = 125k/200.  200 = 60Hz * 3
	PR2 = 208;
	// Enable Low Interrupt
	TMR2IE = 1;
	TMR2IF = 0;
	TMR2IP = 0;
}

void error()
{
	set_dis
}

float get_temp()
{
	
}

char sample_inputs()
{
	LED_CATHODE = 1;
	TRISB = 0xFF;
	return PORTB;
}

void set_LEDs(char mask)
{
	TRISB = 0xFF;
	LED_CATHODE_TRIS = 0;
	LED_CATHODE = 0;
}

digit num_to_disp()
{
	
}

void set_disp(digit disp[2])
{
	TRISB = 0xFF;
	LED_CATHODE = 1;
	DISP_1_TRIS = 0;
	DISP_2_TRIS = 0;
	DISP_1 = 0 & BIT0;
	DISP_2 = 0 & BIT1;
	
}

void interrupt high_priority ISR_high()
{
	if (TMR0IE && TMR0IF)
	{
		TMR0IF = 0;
		return;
	}
}

void interrupt low_priority ISR_low()
{
	
}