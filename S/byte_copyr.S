.text
.globl byte_copyr
.type   byte_copyr, @function
byte_copyr:
	pushl   %edi
        pushl   %esi
        movl    12(%esp), %edi
        movl    20(%esp), %esi
        movl    16(%esp), %ecx

        decl    %edi
        decl    %esi
        addl    %ecx, %edi
        addl    %ecx, %esi

        std
	rep	movsb
	cld

        popl    %esi
        popl    %edi
        ret
