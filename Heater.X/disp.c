#include "disp.h"
#include <xc.h>

// Convert a number (or other char) to display vals
// Can convert single or double digit numbers or the letter E/e

unsigned short num_to_disp(char num)
{
    unsigned short ret;
    do
        switch (num)
        {
        case 0 ... 9:
            ret = num_font[num] | (ret << 8);
            break;
        case 'E':
        case 'e':
            ret = (SEG_A | 0 | 0 | SEG_D | SEG_E | SEG_F | SEG_G);
            break;
        default:
            ret = (0);
            break;
        }
        while ((num = num / 10) >= 10);
    return ret;
}

// Display a single digit of the display
// Don't forget to turn off LEDs before calling
// Digits are 0 or 1

void set_digit(unsigned char dig)
{
    // Output now
    /*
     * Don't do this becaues we have a function that does it for us.  Also,
     * don't change io's out of the main file so we don't forget about them
    DISP_A_TRIS = 0;
    DISP_B_TRIS = 0;
    DISP_C_TRIS = 0;
    DISP_D_TRIS = 0;
    DISP_E_TRIS = 0;
    DISP_F_TRIS = 0;
    DISP_G_TRIS = 0;
     */
    // Select which digit we show.  They're active low
    /*
     * Same reason
    DISP_1 = num;
    DISP_2 = !num;
     */
    // Need to do this long hand because DISP_A thru DISP_G aren't necessarily linear
    if (dig & BIT7) // Blank bit
        dig = 0;
    // Division trick again because these masks are powers of two!
    DISP_A = (dig / DISP_A_MASK);
    DISP_B = (dig / DISP_B_MASK);
    DISP_C = (dig / DISP_C_MASK);
    DISP_D = (dig / DISP_D_MASK);
    DISP_E = (dig / DISP_E_MASK);
    DISP_F = (dig / DISP_F_MASK);
    DISP_G = (dig / DISP_G_MASK);
}
