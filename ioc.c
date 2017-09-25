#include "ioc.h"
#include "io.h"

char * getFrameBuffer()
{
	return (char *)0x000B8000;
}

int write( char *buf, unsigned int len )
{
	int pos = 1920;
	unsigned int i;
	for( i = 0; i < len; ++i )
	{
		if (pos >= 2000)
		{
			move_lines_up();
			pos = 1920;
		}
		write_cell( pos++, buf[i], 15 );
	}
	move_lines_up();
	fb_move_cursor( 1921 );
	write_cell( 1920, '>', 15 );
	return i;
}

void move_lines_up()
{
	int l;
	for( l = 1; l < 25; ++l )
	{
		move_line_up(l);
	}
}

void move_line_up( int l )
{
	if(l==0)
		return;
	
	int c;
	char tmpChar, tmpColor;
	for( c = 0; c < 80; ++c )
	{
		read_cell( lctp(l,c), &tmpChar, &tmpColor );
		write_cell( lctp(l,c), 32, 15 );
		write_cell( lctp(l-1,c), tmpChar, tmpColor );
	}
}

int lctp( int l, int c )
{
	return (l%25)*80 + c;
}

void clear_screen()
{
	int pos;
	for( pos = 0; pos < 2000; ++pos )
	{
		write_cell( pos, 32, 15 );
	}
}

void read_cell( int pos, char * character, char * color )
{
	char * fb = getFrameBuffer(); 
	*character = fb[pos*2];
	*color = fb[pos*2+1];
}

void write_cell( int pos, char character, char color )
{
	char * fb = getFrameBuffer(); 
	fb[pos*2] = character;
	fb[pos*2+1] = color;
}


#define FB_COMMAND_PORT         0x3D4
#define FB_DATA_PORT            0x3D5

/* The I/O port commands */
#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15

/** fb_move_cursor:
*  Moves the cursor of the framebuffer to the given position
*
*  @param pos The new position of the cursor
*/
void fb_move_cursor(unsigned short pos)
{
	outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
	outb(FB_DATA_PORT,    ((pos >> 8) & 0x00FF));
	outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
	outb(FB_DATA_PORT,    pos & 0x00FF);
}

/* The I/O ports */

/* All the I/O ports are calculated relative to the data port. This is because
 * all serial ports (COM1, COM2, COM3, COM4) have their ports in the same
 * order, but they start at different values.
 *	 
 *		See - https://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programmin
 *			for detail of register values.
 */

#define SERIAL_COM1_BASE									0x3F8		/* COM1 base port*/
#define SERIAL_DATA_PORT(base)						(base)
#define SERIAL_FIFO_COMMAND_PORT(base)		(base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)		(base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base)		(base + 4)
#define SERIAL_LINE_STATUS_PORT(base)			(base + 5)

void serial_write( char message )
{
	serial_configure_baud_rate(SERIAL_COM1_BASE,2);
	serial_configure_line(SERIAL_COM1_BASE);
	serial_configure_buffer(SERIAL_COM1_BASE);
	modem_configure_buffer(SERIAL_COM1_BASE);
	if(serial_is_transmit_fifo_empty(SERIAL_COM1_BASE) != 0)
	{
		outb(SERIAL_COM1_BASE, message );
	}
}

/* The I/O port commands */

/* SERIAL_LINE_ENABLE_DLAB:
 * Tells the serial port to expect first the highest 8 bits on the data port
 * then the lowest 8 bits will follow
 */

#define SERIAL_LINE_ENABLE_DLAB						0x80

/* serial_configure_baud_rate:
 * Sets the speed of the data being sent. The default speed of a serial
 * port is 115200 bits/s. The argument is a divisor of that number, hence
 * the resulting speed becomes (115200 / divisor) bits/s.
 *
 * @param com			The COM port to configure
 * @param divisor The divisor
 */
void serial_configure_baud_rate( unsigned int com, unsigned short divisor)
{
	outb( SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB );
	outb( SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF );
	outb( SERIAL_DATA_PORT(com), divisor & 0x00FF );
}

/* serial_configure_line:
 * Configures the line of the given serial port. The port is set to have a
 * data length of 8 bits, no parity bits, one stop bit and break control
 * disabled.
 *
 * @param com		The serial port to configure
 */
void serial_configure_line(unsigned short com)
{
	/* Bit:			|7|6|5|4|3|2|1|0|
	 * Content: |d|b|prty |s|dl |
	 * Value:		|0|0|0|0|0|0|1|1| = 0x03
	 */
	outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

/* serial_configure_buffer:
 * Configures the buffer of a given serial port. The port is set to enable
 * FIFO, clear both receiver and transmission FIFO queues and use 14 bytes
 * as the size of the queue
 *
 * @param com		The serial port to configure
 */
void serial_configure_buffer(unsigned short com)
{
	/* Bit:			|7|6|5 |4|3  |2  |1  |0|
	 * Content: |lvl|bs|r|dma|clt|clr|e|
	 * Value:		|1|1|0 |0|0  |1  |1  |1| = 0xC7
	 * 
	 * lvl - How many bytes should be stored in the FIFO buffers
	 * bs  - If the buffers should be 16 or 64 bytes large
	 * r 	 - Reserved for future use
	 * dma - How the serial port data should be accessed
	 * clt - Clear the transmission FIFO buffer
	 * clr - Clear the receiver FIFO buffer
	 * e	 - If the FIFO buffer should be enabled or not
	 */
	outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

/* modem_configure_buffer:
 * Configures the modem of a given serial port. The port is set to disable
 * interupts because we are not receiving any data
 *
 * @param com		The serial port to configure
 */
void modem_configure_buffer(unsigned short com)
{
	/* Bit:			|7|6|5 |4 |3  |2  |1  |0  |
	 * Content: |r|r|af|lb|ao2|ao1|rts|dtr|
	 * Value:		|0|0|0 |0 |0  |0  |1  |1  | = 0x03
	 *
	 * r	 - Reserved
	 * af	 - Autoflow control enabled
	 * lb	 - Loopback mode (used for debugging serial ports)
	 * ao2 - Auxiliary output 2, used for receiving interrupts
	 * ao1 - Auxiliary output 1, usually disconnected but it is still used in
	 * 				some boards to switch crystals between 1.8432MHZ and 4MHZ
	 *				frequencies used for MIDI
	 * 					1.8432MHZ = 115200/62500 - UART clock; allows integer division
	 *						of common baud rates
	 *						16   x 115200 baud
	 *						192  x 9600 	 baud
	 *						1563 x 1200 	 baud
	 *					4MHZ = 115200/28800 - Common frequency for microprocesses
	 * rts - Ready to transmit
	 * dtr - Data terminal ready
	 */
	outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}

/* serial_is_transmit_fifo_empty:
 * Checks if the transmit FIFO is empty or not for given COM port
 *
 * @param		com		The COM port
 * @return 				0 if the transmit FIFO is not empty
 * 				 				1 if the transmit FIFO is empty
 */
int serial_is_transmit_fifo_empty(unsigned int com)
{
	/* 0x20 = 0010 0000 */
	return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

