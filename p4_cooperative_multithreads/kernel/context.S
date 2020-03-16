// contextSwitch(long* saveArea, long* restoreArea)
    .global contextSwitch

contextSwitch:
    mov 4(%esp),%eax    # move the content at address esp + 4 (saveArea) to eax -- saveArea
    mov 8(%esp),%ecx    # move the content at address esp + 8 (restoreArea) to ecx -- restoreArea

    mov %ebx,0(%eax)    # move ebx to address eax (saveArea[0])
    mov %esp,4(%eax)    
    mov %ebp,8(%eax)
    mov %esi,12(%eax)
    mov %edi,16(%eax)

    movl $1, 20(%eax)

    mov 0(%ecx),%ebx    # restore content at ecx (restoreArea[0]) to ebx
    mov 4(%ecx),%esp
    mov 8(%ecx),%ebp
    mov 12(%ecx),%esi
    mov 16(%ecx),%edi
    

    ret # pop a return address off the stack (esp) and jump to it
        # We want it to return at thread_entry
        # so esp should point at the thread_entry !!


// switchReg2Me(long* saveArea)
    .global switchReg2Me
switchReg2Me:
    mov 4(%esp),%eax    # move the content at address esp + 4 (saveArea) to eax -- saveArea
    mov %ebx,0(%eax)    # move ebx to address eax (saveArea[0])
    mov %esp,4(%eax)    
    mov %ebp,8(%eax)
    mov %esi,12(%eax)
    mov %edi,16(%eax)

    ret

// switchCand2Reg(long* restoreArea)
    .global switchCand2Reg
switchCand2Reg:
    mov 4(%esp),%ecx    # move the content at address esp + 8 (restoreArea) to ecx -- restoreArea
    mov 0(%ecx),%ebx    # restore content at ecx (restoreArea[0]) to ebx
    mov 4(%ecx),%esp
    mov 8(%ecx),%ebp
    mov 12(%ecx),%esi
    mov 16(%ecx),%edi

    ret