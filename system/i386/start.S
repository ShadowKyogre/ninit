.text	
.global _start
_start:
	popl	%ecx			/* %ecx = argc */
	movl	%esp,%esi		/* %esi = argv */
	pushl	%ecx
	leal	4(%esi,%ecx,4),%eax	/* %eax = envp = (4*ecx)+%esi+4 */

	pushl	%eax
	pushl	%esi
	pushl	%ecx
	movl	%eax,environ

#ifdef WANT_SYSENTER
	movl	%eax,%edx
	xorl	%esi,%esi
1:
	add	$4,%edx
	cmpl	%esi,-4(%edx)
	jne	1b
1:
	movl	(%edx),%edi
	test	%edi,%edi
	jz	1f
	addl	$8,%edx
	cmpl	$32,%edi
	jne	1b
	movl	-4(%edx),%edi
	movl	 %edi,__vsyscall
1:
#endif

	call	main
	pushl	%eax
	call	exit
	hlt	/* die now ! will ya ... */
.size	 _start,.-_start
