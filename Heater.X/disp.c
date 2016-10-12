#include <xc.h>

#include "disp.h"
#include <stdlib.h>

// Convert a number (or other char) to disp masks
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

// Display a single digit of the display
// Don't forget to turn off LEDs before calling
// Digits are 0 or 1
void set_digit(unsigned char dig, char num)
{
	// Output now
	DISP_A_TRIS = 0;
	DISP_B_TRIS = 0;
	DISP_C_TRIS = 0;
	DISP_D_TRIS = 0;
	DISP_E_TRIS = 0;
	DISP_F_TRIS = 0;
	DISP_G_TRIS = 0;
	// Select which digit we show.  They're active low
	DISP_1 = num;
	DISP_2 = !num;
	// Need to do this long hand because DISP_A thru DISP_G aren't necessarily linear
	DISP_A = 0 != (dig & DISP_A_MASK);
	DISP_B = 0 != (dig & DISP_B_MASK);
	DISP_C = 0 != (dig & DISP_C_MASK);
	DISP_D = 0 != (dig & DISP_D_MASK);
	DISP_E = 0 != (dig & DISP_E_MASK);
	DISP_F = 0 != (dig & DISP_F_MASK);
	DISP_G = 0 != (dig & DISP_G_MASK);
}
