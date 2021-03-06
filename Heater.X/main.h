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
// 5  -- BUZZER
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
// New Mapping:
// 1  -- MCLRE
// 2  -- Therm
// 3  -- IR_IN
// 4  -- LED_CATHODE
// 5  -- DISP_1
// 6  -- DISP_2
// 7  -- BUZZER
// 8  -- Vss
// 9  -- NC
// 10 -- LED1, DISP_G
// 11 -- FAN
// 12 -- HEAT1
// 13 -- HEAT2
// 14 -- Vdd
// 15 -- NC
// 16 -- NC
// 17 -- TX?
// 18 -- RX?
// 19 -- Vss
// 20 -- Vdd
// 21 -- KEY1, LED_F, DISP_D
// 22 -- KEY2, LED2, DISP_F
// 23 -- KEY3, LED3, DISP_A
// 24 -- KEY4, LED4
// 25 -- KEY5, LED5
// 26 -- KEY6, LED6
// 27 -- Program Clock
// 28 -- Program Data

//                 +-----+
//           MCLRE-|1  28|-PGD
//           Therm-|2  27|-PGC
//           IR_in-|3  26|-KEY6,LED6
//          LEDCth-|4  25|-KEY5,LED5
//           DISP1-|5  24|-KEY4,LED4
//           DISP2-|6  23|-KEY3,LED3,DISP_A
//            BUZZ-|7  22|-KEY2,LED2,DISP_F
//             Vss-|8  21|-KEY1,LED_F,DISP_D
//                -|9  20|-Vdd
//     DISP_G,LED1-|10 19|-Vss
//             FAN-|11 18|-
//           HEAT1-|12 17|-
//           HEAT2-|13 16|-
//             Vdd-|14 15|-
//                 +-----+
// Conn out from front board:
//
//     +---+
// B --| . <  <-- therm
// o --| . |  <-- Gnd
// a --| . |  <-- +5V
// r --| . |  <-- HEAT 2
// d --| . |  <-- HEAT 1
//   --| . <  <-- FAN
//     +---+

// Heat 1, 2, and fan are all 5V active.

typedef enum {
    _OFF = 0,
    _ERR,
    _IDLE,
    _TIMER_SET,
    _TIME_SET,
} global_state;

typedef enum {
    _NONE,
    _FAN,
    _HEAT_ONE,
    _HEAT_BOTH,
} heat_state;

typedef enum {
    _COOL = 0b1,
    _HEAT = 0b10,
} heat_or_cool;

typedef enum {
    _HIGH_HOUR,
    _HIGH_TEMP,
    _LOW_HOUR,
    _LOW_TEMP,
} timer_set_state;

// Ensure all are consecutive numbers because we need to iterate through

typedef enum {
    _KEYS = 0,
    _LEDS = 1,
    _DISP1 = 2,
    _DISP2 = 3,
} scan_state;


#ifndef _BITS_
#define _BITS_
#define BIT0 0b1
#define BIT1 0b10
#define BIT2 0b100
#define BIT3 0b1000
#define BIT4 0b10000
#define BIT5 0b100000
#define BIT6 0b1000000
#define BIT7 0b10000000
#endif

// Define keys first (Give us the cause of the order)
// First six bits of PORTB used because pullups and
// The last two bits are for programming only
#define KEY1 PORTBbits.RB0
#define KEY1_TRIS TRISB0
#define KEY1_MASK BIT0
#define KEY2 PORTBbits.RB1
#define KEY2_TRIS TRISB1
#define KEY2_MASK BIT1
#define KEY3 PORTBbits.RB2
#define KEY3_TRIS TRISB2
#define KEY3_MASK BIT2
#define KEY4 PORTBbits.RB3
#define KEY4_TRIS TRISB3
#define KEY4_MASK BIT3
#define KEY5 PORTBbits.RB4
#define KEY5_TRIS TRISB4
#define KEY5_MASK BIT4
#define KEY6 PORTBbits.RB5
#define KEY6_TRIS TRISB5
#define KEY6_MASK BIT5

#define LONG_PRESS 10

#define LED_CATHODE LATA2
#define LED_CATHODE_TRIS TRISA2

// LED1 isn't on a button, so move it to another port
// so we can use RB6 and RB7 for programming
// Also, RA6 is only an output
#define LED1 LATA6
#define LED1_TRIS TRISA6
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

// pulled high by 1.00K and connected to ground with 104 cap
#define THERM AN0
#define THERM_TRIS TRISA0
// Constants for converting the voltage to temperature, 
// Guessing this because it's most common
#define TEMP_R_DIV (1.00e3)
#define TEMP_R_ZERO (100)

#define TEMP_WINDOW 2

//#define RTD
#define THERMISTER

#ifdef RTD
// Constants for equation for temp for platinum
#define TEMP_A (3.9083e-3)
#define TEMP_B (-5.775e-7)
// Not used when measuring positive voltages
#define TEMP_C (-4.183e-12)
#endif

#ifdef THERMISTER
// Take three measurements of temp vs resistance and then use those to solve for A, B, and C
#define TEMP_A (0)
#define TEMP_B (0)
#define TEMP_C (0)
#endif

// We need to do RC5 protocol magic on this.
// Can we use the serial module?  Probably not
#define IR_IN PORTAbits.RA1
#define IR_IN_TRIS TRISA1

#define BUZZER LATA5
#define BUZZER_TRIS TRISA5
// Not sure if inverted logic...
#define BUZZ_ON 1
#define BUZZ_OFF 0
// These will be the different buzz lengths
#define BUZZ_SHORT 2
#define BUZZ_LONG 4

// Our three outputs that we need to care most about...
#define FAN LATC0
#define FAN_TRIS TRISC0
#define HEAT1 LATC1
#define HEAT1_TRIS TRISC1
#define HEAT2 LATC2
#define HEAT2_TRIS TRISC2

// Run for 30 seconds after heat is off
#define FAN_FEED_HEAT 30
// Run for one second if we are in fan mode
#define FAN_FEED_NO_HEAT 1

// Use an enum to prevent address collisions

typedef enum {
    EE_TIME_ADDR = 0,
    EE_WANT_TEMP_ADDR,
    EE_DAY_TIME_ADDR,
    EE_DAY_TEMP_ADDR,
    EE_NIGHT_TIME_ADDR,
    EE_NIGHT_TEMP_ADDR,
    EE_STATE_ADDR,
} eeprom_addrs;

#endif	/* MAIN_H */

