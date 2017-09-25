#include "ioc.h"
#include "gdt.h"

void kmain()
{
	gdt_install();	
	clear_screen();

	char buf[19] = "Wake up Mr Freeman!";
	write( buf, 19 );
}
