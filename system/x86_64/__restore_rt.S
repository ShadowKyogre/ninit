.text
.align 16
.global __restore_rt
.type __restore_rt,@function
__restore_rt:
	movq	$15, %rax
	syscall
	hlt
