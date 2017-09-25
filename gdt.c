#include "gdt.h"

struct gdt 
{
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));

struct gdt_entry
{
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_middle;
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt _gp;

extern void _gdt_flush();

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
	/* Setup the descriptor base access */
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	/* Setup the descriptor limits */
	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F);

	/* Finally, set up the granularity and access flags */
	gdt[num].granularity |= (gran & 0xF0);
	gdt[num].access = access;
}

void gdt_install()
{
	/* Setup the GDT pointer and limit */
	_gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
	_gp.base = (unsigned long int)&gdt;

	/* Our NULL descriptor */
	gdt_set_gate(0, 0, 0, 0, 0);

	/* The second entry is our Code Segment.  The base address
	 * is 0, the limit is 4 gigabytes, it uses 4 kilobyte
	 * granularity, uses 32-bit opcodes, and is a Code Segment
	 * descriptor.  Please check the table above in the tutorial
	 * in order to see exactly what each value means */
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

	/* The third entry is our Data Segment.  It's exactly the
	 * same as our code segment, but the descriptor type in
	 * this entry's access byte says it's a Data Segment */
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	/* Install the user mode segments into the GDT */
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

	/* Flush our the old GDT / TSS and install the new changes! */
	_gdt_flush();
}
