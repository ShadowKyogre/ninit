.globl str_len
.type str_len, @function
str_len:
	movl	4(%esp), %ecx
	orl	$-1, %eax
.L1:
	incl	%eax
	cmpb	$0, (%ecx, %eax)
	jne	.L1
	
	ret
