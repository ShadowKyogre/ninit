.globl str_chr
.type str_chr, @function
str_chr:
	pushl	%esi
	movl	8(%esp), %esi
	movb	12(%esp), %ah
.L1:
	lodsb
	cmpb	%ah,%al
	je	.L2
	testb	%al,%al
	jne	.L1
.L2:	
	leal	-1(%esi), %eax
	subl	8(%esp), %eax
	popl	%esi
	ret
