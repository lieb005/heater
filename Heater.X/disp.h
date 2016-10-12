#ifndef DISP_H
#define DISP_H

// Function Prototypes
unsigned char num_to_disp(char num);
void set_digit(unsigned char dig, char num);

#ifndef BITS
#define BITS

#define BIT0 0b1
#define BIT1 0b10
#define BIT2 0b100
#define BIT3 0b1000
#define BIT4 0b10000
#define BIT5 0b100000
#define BIT6 0b1000000
#define BIT7 0b10000000
#endif

// Pin configurations
#define DISP_A LATB2
#define DISP_A_TRIS TRISB2
#define DISP_A_MASK BIT0
#define DISP_B LATB5
#define DISP_B_TRIS TRISB5
#define DISP_B_MASK BIT1
#define DISP_C LATB4
#define DISP_C_TRIS TRISB4
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
#define DISP_G LATA6
#define DISP_G_TRIS TRISA6
#define DISP_G_MASK BIT6

#define DISP_1 LATA3
#define DISP_1_TRIS TRISA3
#define DISP_2 LATA4
#define DISP_2_TRIS TRISA4

// Font stuffs
#define A BIT0
#define B BIT1
#define C BIT2
#define D BIT3
#define E BIT4
#define F BIT5
#define G BIT6

const unsigned char font[] =
{
	A | B | C | D | E | F | 0, //0
	0 | B | C | 0 | 0 | 0 | 0, //1
	A | B | 0 | D | E | 0 | G, //2
	A | B | C | D | 0 | 0 | G, //3
	0 | B | C | 0 | 0 | F | G, //4
	A | 0 | C | D | 0 | F | G, //5
	A | 0 | C | D | E | F | G, //6 (hooked top)
	A | B | C | 0 | 0 | 0 | 0, //7
	A | B | C | D | E | F | G, //8
	A | B | C | 0 | 0 | F | G, //9 (straight bottom)
};

#endif