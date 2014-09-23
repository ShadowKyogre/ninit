.globl str_diff
.type	str_diff, @function
str_diff:
	pushl	%edi
	pushl	%esi
	movl	12(%esp), %esi
	movl	16(%esp), %edi
.L1:	
	lodsb
	scasb
	jne	.L2
	testb	%al,%al
	jne	.L1
	xorl	%eax,%eax
	jmp	.L3
.L2:	
	sbbl	%eax,%eax
	orb	$1,%al
.L3:
	popl	%esi
	popl	%edi
	ret
