.text
.global mmap
.type	mmap,@function
mmap:
	mov	$__NR_mmap,%al
	lea	0x4(%esp,1),%edx
	push	%edx
	call	__unified_syscall
	pop	%ecx
	ret
.size mmap,.-mmap
