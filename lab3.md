# lab3
Lab3 aims to implement the user-mode processes and setup the software interrupts (aka. exception). 

## Exercise 1
Allocate spaces for the kernel data structure `envs` array to store the processes' status, entry address, unique id, etc. This array should be mapped user read-only at `UENVS` virtual address.

## Exercise 2
1. initialize the `envs` array.
2. allocate and setup the page directory for each process.
3. implement the method `region_alloc` to allocate pages for users' processes.
4. implement the method `load_icode` to read the `elf` process to the designated address.
5. create and run the user process.

### Initialization
1. set each process as being `FREE`.
2. Put them into the `free_env_list` (just like how we linked the free pages).

### PageDir Setup
1. allocate a new page to accomodate the process's page directory.
2. copy the kernel directory to the new directory. Note that, from `UTOP` to 2^32, all the entries are the same in the two directory.
3. the entries below `UTOP` should be intialized to zero by now.
4. The directory should be placed at `UVPT`.

### Allocating Memory: region_alloc
1. Allocate a new physical page. (`page_alloc`)
2. insert the page to the page table entry. (`pgdir_walk`).
3. the page should be readable and writable by the users and the kernel.

### Loading the ELF
1. the user processes are linked into the kernel image.
2. Given the address to the header, we can find the beginning of the program segments and the end of that.
3. Read all segment to the specified virtual address, which is recorded in the header. Note that, before reading, the memory at the address should be allocated (via `region_alloc`).
4. Before reading the program segments, we should change the kernel page directory to the user's page directory because the program segments are going to be placed in the user's address space.
5. Set the process's entry point.
6. Replace back to the kernel page directory.

### Create and Run User's process
1. allocate a new process.
2. set the status as `ENV_RUNNING`.
3. change the page directory.
4. enter the user process

## Exercise 4 to 6

### Trap Procedure
1. The interrupt happens at some time.
2. The stack is automatically changed by the processor. The stack space and the data segment is specified in the `GD_TSS0` descriptor. The priviledge level is changed to 0. The `ltr` instruction loads the selector of this descriptor into the `Task Register`, which controls the stack replacement before handling the exception.
3. The processor automatically pushes the `%ss`, `%esp`, `eflags`, `%cs`, and `%eip` to the current stack. The `error code` is optionally pushed.
4. Invoke the interrupt handler set in the `IDT` according to the trap number.
5. All the interrupt handler would execute the  `alltraps` procedure first to push the trap frame into the stack, including the `ds`, `es`, `esp` and all the general data registers. Also, the `ds` and `es` segment register should point to kernel data segment.
6. The `trap` function is called and dispatches the task to the specified handler according to the trap number.
7. Interrupt handlers should have a correct `dpl` to ensure security. (some exceptions cannot be invoked by users).

## Exercise 7 to 8

### Syscall
1. Register the handler to the `IDT`.
2. The `syscall` procedure would place the `syscall num` at the `eax` register and other arguments at some genral registers. These registers would be pushed to the stacks in `alltraps`.
3. Dispatch the `syscall` to the specified handler.

## Exercise 9 to 10

### User Memory Check
1. When the user invokes a `syscall` and wants to access a specified memory space, it's curcial to check whether the user has the permission. (e.g. the kernel has permission but the user does not, then the `syscall` function would not throw an exception).
2. Checking the given virtual address and length page by page. (in `kern/pmap.c:user_mem_check`).
