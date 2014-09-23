.globl str_copyn
.type str_copyn, @function
str_copyn:
	pushl	%edi
	pushl	%esi
	movl	12(%esp), %edi
	movl	16(%esp), %esi
	movl	20(%esp), %ecx
.L1:
	decl	%ecx
	js	.L2
	lodsb
	stosb
	testb	%al,%al
	jne	.L1
	decl	%edi
.L2:		
	movl	%edi, %eax
	subl	12(%esp), %eax
	popl	%esi
	popl	%edi
	ret
