	# outb(int port, int val)
	.global outb
outb:
	push %edx
	mov 8(%esp),%dx
	mov 12(%esp),%al
	outb %al,%dx
	pop %edx
	ret

	# outl(int port, int val)
        .global outl
outl:
        mov 4(%esp),%dx
        mov 8(%esp),%eax
        outl %eax,%dx
        ret

	# int inb(int port)
	.global inb
inb:
	push %edx
	mov 8(%esp),%dx
	inb %dx,%al
	pop %edx
	and $0xff,%eax
	ret

	# unsigned long inl(int port)
	.global inl
inl:
	push %edx
	mov 8(%esp),%dx
	inl %dx,%eax
	pop %edx
	ret

    .globl rdmsr
    # uint64_t rdmsr(uint32_t id)
rdmsr:
    mov 4(%esp), %ecx
    rdmsr
    ret

    .globl wrmsr
    # wrmsr (uint32_t id, uint64_t value)
wrmsr:
    mov 4(%esp), %ecx
    mov 8(%esp), %eax
    mov 12(%esp), %edx
    wrmsr
    ret

    .global spuriousHandler_
spuriousHandler_:
    iret

    .extern apitHandler
    .global apitHandler_
apitHandler_:
    // TODO: XMM, MMX, FP, ...
    pusha
    pushf
    call apitHandler
    popf
    popa
    iret

    .global sti
sti:
    sti
    ret

    .global cli
cli:
    cli
    ret

    .global getFlags
getFlags:
    pushf
    pop %eax
    ret

	/* memcpy(void* dest, void* src, size_t n) */
        .global memcpy
memcpy:
        mov 4(%esp),%eax       # dest
        mov 8(%esp),%edx       # src
        mov 12(%esp),%ecx      # n
        push %ebx
1:
        add $-1,%ecx
        jl 1f
        movb (%edx),%bl
        movb %bl,(%eax)
        add $1,%edx
        add $1,%eax
        jmp 1b
1:
        pop %ebx
        ret

     /* bzero(void* dest, size_t n) */
    .global bzero
bzero:
    mov 4(%esp),%eax       # dest
    mov 8(%esp),%ecx      # n
1:
    add $-1,%ecx
    jl 1f
    movb $0,(%eax)
    add $1,%eax
    jmp 1b
1:
    ret

