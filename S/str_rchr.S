.globl str_rchr
.type str_rchr, @function
str_rchr:
	pushl	%esi
	movl	8(%esp), %esi
	movb	12(%esp), %ah
	xorl	%ecx, %ecx
.L1:
	lodsb
	cmpb	%ah,%al
	jne	.L0
	movl	%esi, %ecx
.L0:
	testb	%al,%al
	jne	.L1

	testl	%ecx, %ecx
	jnz	.L3
	movl	%esi, %ecx
.L3:
	subl	8(%esp), %ecx
	leal	-1(%ecx), %eax
	popl	%esi
	ret
