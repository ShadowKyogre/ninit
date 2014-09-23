#ifdef WANT_SYSENTER
.data
.type __vsyscall,@object
.global __vsyscall
__vsyscall:
.Lvsyscall:
.long	.Lcallint80
#endif

.text
.weak exit
exit:
.global _exit
.type	_exit,@function
_exit:
	movb	$1,%al
.size _exit,.-_exit

.global __unified_syscall
.type	__unified_syscall,@function
__unified_syscall:
	movzbl	%al, %eax
.size __unified_syscall,.-__unified_syscall

.global __unified_return
.type	__unified_return,@function
__unified_return:
	push	%edi
	push	%esi
	push	%ebx
	movl	%esp,%edi
	/* we use movl instead of pop because otherwise a signal would
	   destroy the stack frame and crash the program, although it
	   would save a few bytes. */
	movl	0x10(%edi),%ebx
	movl	0x14(%edi),%ecx
	movl	0x18(%edi),%edx
	movl	0x1c(%edi),%esi
	movl	0x20(%edi),%edi
#ifdef WANT_SYSENTER
	call	*.Lvsyscall /* 0xffffe000 */
#else
	int	$0x80
#endif
	cmp	$-124,%eax
	jb	.Lnoerror
	neg	%eax
#ifdef WANT_THREAD_SAFE
	movl	%eax,%ebx
	call	__errno_location
	movl	%ebx,(%eax)
	orl	$-1,%eax
#else
	mov	%eax,errno
	sbb	%eax,%eax               # eax = eax - eax - CY = -1
#endif
.Lnoerror:
	pop	%ebx
	pop	%esi
	pop	%edi
	ret
.size __unified_return,.-__unified_return

#ifdef WANT_SYSENTER
.Lcallint80:
	int	$0x80
	ret
#endif
