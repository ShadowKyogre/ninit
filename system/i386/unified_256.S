.text
.global __unified_syscall_256
.type __unified_syscall_256,@function
__unified_syscall_256:
	movzwl %ax,%eax
	jmp __unified_return
.size	__unified_syscall_256,.-__unified_syscall_256
