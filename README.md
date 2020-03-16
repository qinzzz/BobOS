# BobOS: Operating System

- atomic.h:
    Define atomic and atomicPtr classes using built-in '__atomic' functions. 
    It introduces the abstraction of atomic operation, which is an operation that can be excuted without others' interfere.

- config.cc & config.h:
    Discover the configuration of the system, like memory size, number of processors, and infomation about APIC.
    It introduces the abstraction and manages the resource of SDT(System Description Table), APIC(Advanced Programmable Interrupt Controller), 
    MADT(Multiple APIC Description Table), and RSD(Root System Descriptior).

- critical.cc & critical.h:
    It introduces abstractions for the entry and exit of a critical section. Those functions help to protect a critical section from concurrent access.

- debug.cc & debug.h:
    Define a class Debug, which provides some functions for debugging, like to print results, report kernel panics and shutdown cores.
    It introduces the abstraction of debug functions.

- idt.cc & idt.h:
    IDT is used by the hardware to determine where the interrupt handlers are located.
    It introduces the abstraction of an Interrupt Descriptor Table, including the interrupt gate and trap gate. Also manages the resourec of IDT.

- init.cc & init.h: 
    Initialize the kernel.
    It manages the resource of a UART, discover the configuration of system, start all the processors, and call kernelMain() to run tests.

- io.h:
    It introduces the abstraction of inputstream and outputstream.

- kernel.cc & kernel.h:
    It is an symbolic linked to the test case that contains kernelMain();
    It is an abstraction of OS kernel.

- libk.cc & libk.h:
    It introduces the abstraction of namespace K that use formatted snprintf to print strings.

- machine.S && machine.h:
    Define basic funcitons to write/read data from I/O ports and MSRs.
    It manages the resource of machine's I/O ports.

- Makefile:
    It includes the commands to compile and link our program.

- mbr.S:
    The bootstrp code for the kernel.  
    It introduce the abstraction of Task State Segment (TSS) and Global Descriptor Table (GDT).
    It manages the resource of MBR, a bootsector of a hard disk.

- script.ld:
    It decribes that MBR will be compiled at 0x7c00, and the kernel will be loaded at 0x8000.

- smp.cc & smp.h:
    SMP -- Symmetric multiprocessing. 
    It involves a multiprocessor computer hardware and software architecture where several identical processors are connected to a single, shared main memory.
    It manages the resource of a symmetric multiprocessing and also one single CPU.

- snprintf.cc & snprintf.h:
    snprintf is a function to write formatted strings to character string buffer. Unlike sprintf, it checks limit for string length.
    It introduced the abstraction of print functions that write formatted strings and limit string length.

- stdint.h:
    Define standard int types used in this program.
    It introduces the abstration of different interger types.

- u8250.cc & u8250.h: 
    8250 UART -- Universal Asynchronous Receiver/Transmitter is a chip for asynchronous serical communication. U8250 is used to send characters over a serial port so QEMU prints them out.


## P1
Bootstrap kernel and critical section. 

## P2&P3
Deal with heap memory allocation. Use header&footer for every memory block and a double linked list to trace all free blocks.
Implement malloc(), free(), new, delete. Tested with space and time efficiency.

## P4
Cooperating multi-threading. A thread calls yield() to switch to another thread.

## P5
Preemptive multi-threading. APIT generates an interrupte every 1ms and calls yield() to switch the threads.
Sycronization primitives. Implement semaphore and use it for barrier, bounded buffer, etc.

## P6
An ext2-like file system. Mount a FS from a disk or initialize on a device. You can read, write, find, and create hard links.