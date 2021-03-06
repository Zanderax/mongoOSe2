#include "io.h"

#define PIC1_PORT_A 0x20
#define PIC2_PORT_A 0xA0

#define PIC1_START_INTERRUPT 0x20
#define PIC2_START_INTERRUPT 0x28
#define PIC2_END_INTERRUPT	 PIC2_START_INTERRUPT + 7

#define PIC_ACK		0x20

/* pic_ack:
 *	acks an interrupt from either PIC 1 or PIC 2
 *	@param num - The number of the interupt
 */

void pic_ack( unsigned integer interrupt)
{
	if(interrupt < PIC1_START_INTERRUPT || interrupt > PIC2_END_INTERRUPT)
	{
		return;
	}

	if(interrupt < PIC2_START_INTERRUPT)
	{
		outb(PIC1_PORT_A, PIC_ACK)
	}
	else
	{
		outb(PIC2_PORT_A, PIC_ACK)
	}

}

#define KBD_DATA_PORT	0x60

/* read_scan_code:
 *	Reads a scan code from keyboard
 *
 * 	@return The scan code (not ASCII)
 */
unsigned char read_scan_code(void)
{
	return inb(KBD_DATA_PORT);
}
