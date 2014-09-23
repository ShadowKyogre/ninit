.globl byte_zero
.type	byte_zero, @function
byte_zero:
	xorb	%al, %al

.globl byte_zero_end 
byte_zero_end:
	pushl	%edi
	movl	8(%esp), %edi
	movl	12(%esp), %ecx

	cld
	rep	stosb

	popl	%edi
	ret
