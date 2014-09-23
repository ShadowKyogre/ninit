.globl str_copy
.type str_copy, @function
str_copy:
	pushl	%edi
	pushl	%esi
	movl	12(%esp), %edi
	movl	16(%esp), %esi
.L1:
	lodsb
	stosb
	testb	%al,%al
	jne	.L1

	leal	-1(%edi), %eax
	subl	12(%esp), %eax
	popl	%esi
	popl	%edi
	ret
