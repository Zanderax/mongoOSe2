global load_idt

;loads the interupt descriptor table
;stack: [esp + 4] the address of the first entry
;		[esp	] the return address
load_idt:
	mov		eax, [esp + 4] ;load address into eax
	lidt	eax			   ;load the idt
	ret


%macro no_error_code_interrupt_handler %1
global interrupt_handler_%1
interupt_handler_%1
	push dword 0					;push 0 as error code
	push dword %1					;push interupt number
	jmp common_interrupt_handler	;jump to common code
%endmacro

%macro error_code_interrupt_handler %1
global interrupt_handler_%1
interupt_handler_%1
	push dword %1					;push interupt number
	jmp common_interrupt_handler	;jump to common code
%endmacro

common_interupt_handler:
	pushad
	cld /* C code following the sysV ABI requires DF to be clear on function entry */
	call interrupt_handler
	popad
	iret

no_error_code_interupt_handler 0
no_error_code_interupt_handler 1
no_error_code_interupt_handler 2
no_error_code_interupt_handler 3
no_error_code_interupt_handler 4
no_error_code_interupt_handler 5
no_error_code_interupt_handler 6
error_code_interupt_handler 7 
