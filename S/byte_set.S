.globl byte_set
.type	byte_set, @function
byte_set:	
	movb	12(%esp), %al
	jmp	byte_zero_end
