/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
TRAPHANDLER_NOEC(DivideError, T_DIVIDE)
TRAPHANDLER_NOEC(Debug, T_DEBUG)
TRAPHANDLER_NOEC(NMI, T_NMI)
TRAPHANDLER_NOEC(Breakpoint, T_BRKPT)
TRAPHANDLER_NOEC(Overflow, T_OFLOW)
TRAPHANDLER_NOEC(BoundRangeExceed, T_BOUND)
TRAPHANDLER_NOEC(InvalidOpcode, T_ILLOP)
TRAPHANDLER_NOEC(DeviceNotAvailable, T_DEVICE)
TRAPHANDLER(DoubleFault, T_DBLFLT)
/*TRAPHANDLER_NOEC(CopSegOverrun, T_TSS)*/
TRAPHANDLER(InvalidTss, T_TSS)
TRAPHANDLER(SegNotPresent, T_SEGNP)
TRAPHANDLER(StackFault, T_STACK)
TRAPHANDLER(GeneralProtect, T_GPFLT)
TRAPHANDLER(PageFault, T_PGFLT)
/*TRAPHANDLER_NOEC(UnknownTrap, T_RES)*/
TRAPHANDLER_NOEC(FPUFloatErr, T_FPERR)
TRAPHANDLER_NOEC(AlignCheck, T_ALIGN)
TRAPHANDLER_NOEC(MachineCheck, T_MCHK)
TRAPHANDLER_NOEC(SIMDFloatException, T_SIMDERR)
TRAPHANDLER_NOEC(SysCall, T_SYSCALL)

TRAPHANDLER_NOEC(IRQHandler0, IRQ_OFFSET + 0)
TRAPHANDLER_NOEC(IRQHandler1, IRQ_OFFSET + 1)
TRAPHANDLER_NOEC(IRQHandler2, IRQ_OFFSET + 2)
TRAPHANDLER_NOEC(IRQHandler3, IRQ_OFFSET + 3)
TRAPHANDLER_NOEC(IRQHandler4, IRQ_OFFSET + 4)
TRAPHANDLER_NOEC(IRQHandler5, IRQ_OFFSET + 5)
TRAPHANDLER_NOEC(IRQHandler6, IRQ_OFFSET + 6)
TRAPHANDLER_NOEC(IRQHandler7, IRQ_OFFSET + 7)
TRAPHANDLER_NOEC(IRQHandler8, IRQ_OFFSET + 8)
TRAPHANDLER_NOEC(IRQHandler9, IRQ_OFFSET + 9)
TRAPHANDLER_NOEC(IRQHandler10, IRQ_OFFSET + 10)
TRAPHANDLER_NOEC(IRQHandler11, IRQ_OFFSET + 11)
TRAPHANDLER_NOEC(IRQHandler12, IRQ_OFFSET + 12)
TRAPHANDLER_NOEC(IRQHandler13, IRQ_OFFSET + 13)
TRAPHANDLER_NOEC(IRQHandler14, IRQ_OFFSET + 14)
TRAPHANDLER_NOEC(IRQHandler15, IRQ_OFFSET + 15)

/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
	pushw $0 /*padding*/
	pushw %ds /*ds register*/
	pushw $0 /*padding*/
	pushw %es /*es register*/
	pushal /*save all the register*/
	movw $GD_KD, %ax
	movw %ax, %es
	movw %ax, %ds
	pushl %esp /* the pointer to trapframe */
	call trap