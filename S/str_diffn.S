.globl str_diffn
.type	str_diffn, @function
str_diffn:
	pushl	%edi
	pushl	%esi
	movl	12(%esp), %esi
	movl	16(%esp), %edi
	movl	20(%esp), %ecx
.L1:
	decl	%ecx
	js	.L2
	lodsb
	scasb
	jne	.L3
	testb	 %al,%al
	jne	.L1
.L2:	
	xorl	%eax,%eax
	jmp	.L4
.L3:	
	sbbl	%eax,%eax
	orb	$1,%al
.L4:
	popl	%esi
	popl	%edi
	ret
