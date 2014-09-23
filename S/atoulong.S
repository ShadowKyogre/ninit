.globl	atoulong
.type	atoulong,@function
atoulong:
	xorl	%edx,%edx
	xorl	%eax,%eax
	movl	4(%esp),%ecx
	jmp	.L1
.L2:
	imull	$10,%eax,%eax
	addl	%edx,%eax
	incl	%ecx
.L1:
	movb	(%ecx),%dl
	subb	$48,%dl
	cmpb	$9,%dl
	jbe	.L2
	ret
