#include "syscalls.h"

.text
.type   __restore,@function
.global __restore
.align 8
__restore:
	popl %eax
	movl $__NR_sigreturn,%eax
	int $0x80
	hlt	/* die if syscall returns */
.size	__restore,.-__restore
