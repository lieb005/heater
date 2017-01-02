/* 
 * File:   rc5.h
 * Author: talia
 *
 * Created on October 9, 2016, 3:47 PM
 */

#ifndef RC5_H
#define	RC5_H

#include <stdint.h>

// Percentage of freq allowed to be off
// So we can vary by +/-IR_WINDOW*freq
#define IR_WINDOW .01


// Do a table maybe?
// Do a decode function?  Just use interrupts?
//   What would that return?  ENUM of fns
// Then need to make enum, too

// We really ned an interruptable pin for this...  And a timer...
// So far our only way to do this is to share a timer and poll the pin
// Maybe we can use the blink timer?  Is it really necessary?

/*
 * https://en.wikipedia.org/wiki/RC-5
    The command comprises 14 bits:
    A start bit, which is always logic 1 and allows the receiving IC to set the proper gain.
    A field bit, which denotes whether the command sent is in the lower field (logic 1 = 0 to 63 decimal)
        or the upper field (logic 0 = 64 to 127 decimal). The field bit was added
        later by Philips when it was realized that 64 commands per device were insufficient.
        Previously, the field bit was combined with the start bit. Many devices still use this original system.
    A control bit, which toggles with each button press. This allows the receiving device 
        to distinguish between two successive button presses (such as "1", "1" for "11") as
        opposed to the user simply holding down the button and the repeating commands
        being interrupted by a person walking by, for example.
    A five-bit system address, that selects one of 32 possible systems.
    A six-bit command, that (in conjunction with the field bit) represents one of
        the 128 possible RC-5 commands.
 */

#define IR_DEV_MASK (0x07C0L) // 5 bits offset by 6
#define IR_FN_MASK  (0x003FL) // 6 bits

// Each bit is either a high/low transition or a low/high transition
// On our receiver (we hope) no data = high
//
// ---------+  +--+  +-----+  +--+     +-----+  +------------
//          |  |  |  |     |  |  |     |     |  |
//          +--+  +--+     +--+  +-----+     +--+
//
//          |     |     |     |     |     |     |
//  hiiiii   lo hi lo hi hi lo hi lo lo hi hi lo hiiiiiii
//			   0     0     1      1    0     1

typedef enum {
    TV1 = 0,
    TV2 = 1,
    TXT = 2,
    TV_EXT = 3,
    LV = 4,
    VCR1 = 5,
    VCR2 = 6,
    // RESVD = 7,
    SAT1 = 8,
    VCR_EXT = 9,
    SAT2 = 10,
    // RESVD = 11,
    CD_VID = 12,
    CD_PHOTO = 14,
    PREAMP1 = 16,
    TUNER = 17,
    REC1 = 18,
    PREAMP2 = 19,
    CD = 20,
    COMBI = 21,
    SAT = 22,
    REC2 = 23,
    // RESVD = 24,
    // RESVD = 25,
    CDR = 26,
    // RESVD = 27,
    // RESVD = 28,
    LIGHTING = 29,
    // RESVD = 30,
    // RESVD = 31,
} dev_e;

#define IR_VARIABILITY 5
#define IR_LONG_PULSE 20
#define IR_SHORT_PULSE 10

// Basically an index for times.
unsigned char pulse_num = 0;
// This variable contains values that represent the length of each half pulse
// of the ir data
volatile uint32_t ir_times[32]; // Should only need to go to 28, buuuuut


#endif	/* RC5_H */