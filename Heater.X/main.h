/* 
 * File:   main.h
 * Author: talia
 *
 * Created on October 9, 2016, 3:47 PM
 */

#ifndef MAIN_H
#define	MAIN_H

// On board, switches are active low

// Inbuilt IC
//
// 1  -- Vss
// 2  -- NC
// 3  -- KEY3, LED3, DISP_A
// 4  -- NC
// 5  -- ~BUZZER
// 6  -- KEY6, LED_C, DISP_B
// 7  -- KEY1, LED_F, DISP_D
// 8  -- KEY5, LED5, DISP_E
// 9  -- KEY4, LED4, DISP_C
// 10 -- DISP_2 (Active low)
// 11 -- LED1, DISP_G
// 12 -- DISP_1 (Active low)
// 13 -- KEY2, LED2, DISP_F
// 14 -- IR_IN
// 15 -- LED_CATHODE
// 16 -- THERM
// 17 -- HEAT2
// 18 -- HEAT1
// 19 -- FAN
// 20 -- Vdd
//

// Conn out from ctrl board:
//
//   +---+
// --| . <  <-- therm
// --| . |  <-- Gnd
// --| . |  <-- +5V
// --| . |  <-- HEAT 2
// --| . |  <-- HEAT 1
// --| . <  <-- FAN
//   +---+

// Heat 1, 2, and fan are all 5V active.

enum {OFF = 0, IDLE, TIMER_SET};
enum heat {FAN, HEAT1, HEAT2};
enum timer {HIGH_HOUR, HIGH_TEMP, LOW_HOUR, LOW_TEMP};

#define BIT0 0b1
#define BIT1 0b10
#define BIT2 0b100
#define BIT3 0b1000
#define BIT4 0b10000
#define BIT5 0b100000
#define BIT6 0b1000000
#define BIT7 0b10000000


#define LED1 LATB6
#define LED1_TRIS TRISB6
#define LED1_MASK BIT0
#define LED2 LATB1
#define LED2_TRIS TRISB1
#define LED2_MASK BIT1
#define LED3 LATB2
#define LED3_TRIS TRISB2
#define LED3_MASK BIT2
#define LED4 LATB3
#define LED4_TRIS TRISB3
#define LED4_MASK BIT3
#define LED5 LATB4
#define LED5_TRIS TRISB4
#define LED5_MASK BIT4
#define LED_C LATB5
#define LED_C_TRIS TRISB5
#define LED_C_MASK BIT5
#define LED_F LATB0
#define LED_F_TRIS TRISB0
#define LED_F_MASK BIT6

typedef union led_digit_union {
	char segment_bits;
	struct led_digit_struct {
		char A : 1;
		char B : 1;
		char C : 1;
		char D : 1;
		char E : 1;
		char F : 1;
	} segments;
} digit;
digit num_to_disp(char num);

#define LED_CATHODE LATA3
#define LED_CATHODE_TRIS TRISA3

#define DISP_1 LATA4
#define DISP_1_TRIS TRISA4
#define DISP_2 LATA5
#define DISP_2_TRIS TRISA5

#define DISP_A LATB2
#define DISP_A_TRIS TRISB2
#define DISP_A_MASK BIT0
#define DISP_B LATB5
#define DISP_B_TRIS TRISB5
#define DISP_B_MASK BIT1
#define DISP_C LATB3
#define DISP_C_TRIS TRISB3
#define DISP_C_MASK BIT2
#define DISP_D LATB0
#define DISP_D_TRIS TRISB0
#define DISP_D_MASK BIT3
#define DISP_E LATB4
#define DISP_E_TRIS TRISB4
#define DISP_E_MASK BIT4
#define DISP_F LATB1
#define DISP_F_TRIS TRISB1
#define DISP_F_MASK BIT5
#define DISP_G LATB6
#define DISP_G_TRIS TRISB6
#define DISP_G_MASK BIT6

// pulled high by 1.00K and connected to ground with 104 cap
#define THERM AN0
#define THERM_TRIS TRISA0

#define KEY1 RB0
#define KEY1_TRIS TRISB0
#define KEY1_MASK BIT0
#define KEY2 RB1
#define KEY2_TRIS TRISB1
#define KEY2_MASK BIT1
#define KEY3 RB2
#define KEY3_TRIS TRISB2
#define KEY3_MASK BIT2
#define KEY4 RB3
#define KEY4_TRIS TRISB3
#define KEY4_MASK BIT3
#define KEY5 RB4
#define KEY5_TRIS TRISB4
#define KEY5_MASK BIT4
#define KEY6 RB5
#define KEY6_TRIS TRISB5
#define KEY6_MASK BIT5

#define IR_IN RA1
#define IR_IN_TRIS TRISA1

#define HEAT1 LATC0
#define HEAT1_TRIS TRISC0
#define HEAT2 LATC1
#define HEAT2_TRIS TRISC1
#define FAN LATC2
#define FAN_TRIS TRISC2

#endif	/* MAIN_H */

