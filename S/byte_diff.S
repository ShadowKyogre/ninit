.global byte_diff
.type	byte_diff,function
byte_diff:
	pushl	%esi
	pushl	%edi
	xorl	%eax, %eax
	movl	12(%esp), %esi
	movl	16(%esp), %ecx
	movl	20(%esp), %edi
	
	cld
	rep	cmpsb
	jz	.Lout
	sbbl	%eax, %eax
	orl	$1, %eax
.Lout:		
	popl %edi
	popl %esi
	ret
