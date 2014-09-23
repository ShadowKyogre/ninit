.globl byte_copy
.type   byte_copy, @function
byte_copy:
        pushl   %edi
        pushl   %esi
        movl    12(%esp), %edi
        movl    16(%esp), %ecx
        movl    20(%esp), %esi
        cld

#ifdef BYTE_COPY_FAST	
        movl    %ecx, %eax
        shrl    $2, %ecx
        andl    $3, %eax

	rep	movsl
	movl	%eax, %ecx
#endif
	rep	movsb

        popl    %esi
        popl    %edi
        ret
