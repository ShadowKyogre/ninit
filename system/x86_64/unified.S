#define SYS_exit 0x3c

.text
.weak exit
exit:
.global _exit
_exit:
	mov	$SYS_exit,%al

.global __unified_syscall
__unified_syscall:
	movzbl	%al, %eax
	mov	%rcx, %r10
	syscall
	cmpq	$-128, %rax
	jbe	.Lnoerror
	negl	%eax
	pushq	%rax
	call	__errno_location
	popq	%rcx
	movl	%ecx,(%rax)
	orq	$-1, %rax
.Lnoerror:

	ret
.Lhere:
	.size	 __unified_syscall,.Lhere-__unified_syscall
