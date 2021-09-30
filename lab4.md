# lab4

## PartA: setup virtual memory for CPU LAPIC
1. map `MMIOBASE` to hardware's physical memory
2. initialize each processor at `mp_init` and `lapic_init`.
3. The LAPIC units are responsible for delivering interrupts throughout the system.
4. After calling `boot_aps` function, all the processors are waken up.
5. Before waking up other processors, lock the kernel (only one processor can access the kernel).

## PartA: setup stack/TSS for each processor
1. In `mem_init_mp`, map `KSTACKTOP - i *(KSTKSIZE + KSTKGAP)` as each processor's kernel stack.
2. In `trap_init_percpu`, setup TSS for each CPU. Note that, the selector of the each TSS is unique. Different CPU should have different TSS.
3. Because only one processor can access the kernel at a time, processors should lock the kernel when being interrupted by hardwares or exceptions and unlock it when entering the user's process.

## PartA: Round-robin scheduling
1. The `sched_yield` function never returns.
2. The next cursor points to the next process if the current process exists. Otherwise, the cursor starts from 0.
3. If no process is runnable, halt the CPU by calling `sched_halt` and unlock the kernel.

## PartA: simple fork
1. call `env_alloc` to allocate a new process.
2. the status of the new process is set to `ENV_RUNNABLE`.
3. Setting the child's `tf_eax` of its `trapframe` to 0, which indicates the returned value of the system call.

## PartB: User-level page fault handling
1. allocate the exception stack for each process.
2. push the current `trapframe` to that stack as the `UTrapframe`.
3. the user-defined page fault handler deals with the page fault with the information in `UTrapframe`.
4. Pop out the `UTrapframe` to restore the context (in `lib/pentry.S`). Note that, the `eflags` register would be changed after arithmetic calculation on other registers.
5. Use `uvpt` and `uvpd` to access page table entry in user mode [detailed explanation](https://pdos.csail.mit.edu/6.828/2018/labs/lab4/uvpt.html)

## PartB: User-level COW fork
1. Register a `pgfault` function, which copies the page when writing on a `COW` page.
2. `fork` a new process.
3. Set all the `writable` pages as `COW` for the parent and the child. Directly map readable pages to both processes.
4. The exception stack should not be copied.
5. The parent registers a `pgfault` function for the child.

## PartC: enable interrupt
1. Set the `eflags` as `FL_IF` when `env_alloc`ing a new process.
2. All the `trap` gates should be set as `interrupt` gates because the kernel would check the `interrupt` flag is disabled when trapped by a `syscall` or an `exception`. Note that, the `interrupt` gate will re-set the `eflags` register.
3. Handle the clock interrupt and re-schedule the processes.

## PartC: IPC
1. Block the process when it calls `recv` until it receives a message. (its status is set as `ENV_NOT_RUNNABLE`)
2. The process that sends a message would re-set the receiver's status and assign the message to receiver's `env_ipc_value` field.
3. The receiver is marked as `ENV_RUNNABLE` by the sender. Note that, the `recv` syscall never returns. The returned value is directly assigned to its `tf_eax` field as a return.
