#include <asm/unistd.h>

#define syscall_weak(name,wsym,sym) \
.text; \
.type wsym,@function; \
.weak wsym; \
wsym: ; \
.type sym,@function; \
.global sym; \
sym: \
	mov	$__NR_##name,%al; \
	jmp	__unified_syscall

#define syscall(name,sym) \
.text; \
.type sym,@function; \
.global sym; \
sym: \
	mov	$__NR_##name,%al; \
	jmp	__unified_syscall
