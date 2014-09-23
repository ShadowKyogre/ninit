#include <asm/unistd.h>

#define syscall(name,sym) \
.text; \
.type sym,@function; \
.global sym; \
sym: \
.ifle __NR_##name-255; \
	movb $__NR_##name,%al; \
	jmp __unified_syscall; \
.else; \
	movw $__NR_##name,%ax; \
	jmp __unified_syscall_256; \
.endif; \
.Lend##sym: ; \
.size sym,.Lend##sym-sym

#define syscall_weak(name,wsym,sym) \
.text; \
.type wsym,@function; \
.weak wsym; \
wsym: ; \
.type sym,@function; \
.global sym; \
sym: \
.ifle __NR_##name-255; \
	movb $__NR_##name,%al; \
	jmp __unified_syscall; \
.else; \
	movw $__NR_##name,%ax; \
	jmp __unified_syscall_256; \
.endif; \
.Lend##sym: ; \
.size sym,.Lend##sym-sym
