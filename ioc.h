#ifndef IOC_H
#define IOC_H

char * getFrameBuffer();
int write( char * buf, unsigned int len );
void clear_screen();
void read_cell( int pos, char * character, char * color );
void write_cell( int pos, char character, char color );
void fb_move_cursor(unsigned short pos);
void move_lines_up();
void move_line_up( int l );
int lctp( int l, int c ); // Line Column to Position


void fb_move_cursor(unsigned short pos);
void serial_write( char message );
void serial_configure_baud_rate( unsigned int com, unsigned short divisor);
void serial_configure_line(unsigned short com);
void serial_configure_buffer(unsigned short com);
void modem_configure_buffer(unsigned short com);
int serial_is_transmit_fifo_empty(unsigned int com);

#endif //IOC_H
