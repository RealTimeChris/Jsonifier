
section .text
global compareLong

equal:
    mov rax, 1
    ret

notEqual:
    mov rax, 0
    ret

compareShort:  
    cmp r8, 7
    jle .shortComparison

    .whileLoop:
        cmp r8, 8
        jle .finalComparison

        mov rax, [rcx]
        cmp rax, [rdx]
        jne notEqual

        sub r8, 8
        add rcx, 8
        add rdx, 8
        jmp .whileLoop

    .finalComparison:
        sub r8, 8

        sub rcx, 8
        sub rdx, 8

        mov rax, [rcx]
        cmp rax, [rdx]
        jne notEqual
        jmp equal

    .shortComparison:
        mov eax, [rcx]
        cmp eax, [rdx]
        jne notEqual
        add rcx, 4
        add rdx, 4
        sub r8, 4
        cmp r8, 0
        je equal
        mov ax, [rcx]
        cmp ax, [rdx]
        jne notEqual
        add rcx, 2
        add rdx, 2
        sub r8, 2
        cmp r8, 0
        je equal
        mov al, [rcx]
        cmp al, [rdx]
        jne notEqual
        je equal 


compareLong: 

  .checkRemaining_32:
    cmp r8, 32
    jl .checkRemaining_16

    vmovdqu ymm0, [rcx]
    vmovdqu ymm1, [rdx]
    vpcmpeqq ymm0, ymm1
    vpmovmskb eax, ymm0

    cmp eax, 0xFFFFFFFF
    jne notEqual

    add rcx, 32
    add rdx, 32
    sub r8, 32
    cmp r8, 0
    je equal
    jmp .checkRemaining_32

  .checkRemaining_16:
    cmp r8, 16
    jl compareShort

    movdqu xmm0, [rcx]
    movdqu xmm1, [rdx]
    pcmpeqq xmm0, xmm1
    pmovmskb eax, xmm0

    cmp ax, 0xFFFF
    jne notEqual

    add rcx, 16
    add rdx, 16
    sub r8, 16
    cmp r8, 0
    je equal
    jmp compareShort.whileLoop
