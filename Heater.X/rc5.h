/* 
 * File:   rc5.h
 * Author: talia
 *
 * Created on October 9, 2016, 3:47 PM
 */

#ifndef RC5_H
#define	RC5_H

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

typedef enum {} command_e;
typedef enum {} device_e;

#endif	/* RC5_H */