#include "rc5.h"

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

// This function will sample the d
char decode()
{
	// Shift the data in to a variable.  Use a short because 16 bits
	// It'll be right justified (top two bits = 0)
	unsigned short data = 0;
	
	
}