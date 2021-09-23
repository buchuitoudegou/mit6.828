# lab2

## global variable explanation
1. `npages`: the total amount of physical pages.
2. `npages_basemem`: the amount of physical pages of the base memory.
3. `kern_pgdir`: the kernel page directory.
4. `pages`: the array storing all the physical pages.
5. `page_free_list`: the linked list storing the free physical pages.

## Physical Address and PageInfo and Virtual Address
1. all the variables can only be accessed by **virtual address**.
2. the order of pages in the `pages` array preserves the order of the physical pages.
3. Hence, we can calculate the physical address of a page given its virtual address (the index in the array * PGSIZE).
4. The kernel virtual address is simply the physical address plus the `KERNBASE` since the page table maps the `[KERNBASE, KERNBASE + 4MB]` to `[0, 4MB]`.

### Address Translation
```
PA=>VA: KADDR(pa): pa + KERNBASE
VA=>PA: PADDR(va): va - KERNBASE
PageInfo*=>PA: page2pa(page_info): (page_info - pages) * PGSIZE
PA=>PageInfo*: pa2page(pa): &pages[pa / PGSIZE]
PageInfo*=>VA: KADDR(page2pa(page_info))
VA=>PageInfo*: pa2page(PADDR(va))
```

## boot alloc and page init/alloc/free

### boot_alloc
`boot_alloc` is used to allocate memory before setting up the `free_page_list`. Initializing some kernel data structure, such as page directory, `pages` array, needs to allocate memory above the `KERNBASE`. The magic symbol `end` used in the `boot_alloc` represents the first virtual address that is not be assigned. It is intialized automatically by the linker.

When allocating new pages, we need to check whether the newly allocated memory is out of the boundary by comparing the physical address of the next free memory with the maximum physical memory (i.e. npages * PGSIZE).

### page_init
`page_init` initializes the `pages` array which records the state of each physical page. The base physical memory is all free while the memory from `IOPYSMEM` to `EXPHYSMEM` is all in use. The memory from `EXTPHYSMEM` to `npages` is parly used by the kernel (e.g. `kern_pgdir`, some global variables, etc). Because we can know the virtual address of the first free memory in the kernel by calling `boot_alloc(0)`, we can translate it into physical address and compare it with the current physical address. If the current one is lower, then it is in use and otherwise, it is free.

Note that, `EXTPHYSMEM` is set to `0x100000`, which is the physical address of the kernel image (`kern/kernel.ld:15`). All the kernel data structure is on the top of this address.

### page_alloc
1. find one empty page
2. unlink it from the `page_free_list`
3. return the page

### page_free
1. link it to the `page_free_list`

## page lookup/insert/remove from the page table

### pgdir_walk
`pgdir_walk` is the fundamental of the page table lookup procedure. It accepts a page directory, a virtual address, and a `create` flag, and returns a `page table entry pointer`.

The `create` flag indicates whether the function can insert a newly allocated page table to the directory when the page directory entry (i.e. the page table) has not been created. If the page table doesn't exist and cannot be allocated, the `page table entry pointer` will be set to be `NULL`.

Note that, we can check whether the returned value is `NULL` to judge if we successfully find the `page table entry pointer`.

### page_lookup
1. find the `page table entry pointer` by calling `pgdir_walk(pgdir, va, 0)`
2. if the pointer is `NULL` or the content of the pointer is `NULL` (i.e. the page table not exists or the physical page not exists), return `NULL`.
3. Otherwise, store the pointer to `pte_store` and return the `PageInfo` of the physical page.

### page_insert
1. find the `page table entry pointer` by calling `pgdir_walk(pgdir, va, 1)`
2. if the pointer is `NULL` (i.e. creating page table fails), return error.
3. increment the page reference bit.
4. remove the page if the pointer has pointed to a physical page.
5. assign the current page to the pointer

### page_remove
1. find the `page table entry pointer` by calling `page_lookup(pgdir, va, &pgt_entry)`
2. if the page does not exist, silently return.
3. Otherwise, derefer the page.
4. set the pointer to `NULL`.
5. invalidate the `TLB` entry.

## setting up the kernel's page directory

### boot_map_region
1. map the `[va, va+size)` to `[pa, pa+size)`.
2. for each va, find its `page table entry pointer`.
3. let the pointer point to the corresponding `pa`.
4. `va += PGSIZE, pa += PGSIZE`

### map pages to UPAGES
1. `boot_map_region(kern_dir, UPAGES, UVPT - UPAGES, PADDR(pages), PTE_U|PTE_P)`

### map kernel stack
1. `boot_map_region(kern_pgdir, KSTACKTOP-KSTKSIZE, KSTKSIZE, PADDR(bootstack), PTE_P | PTE_W)`

### map other kernel space
1. `boot_map_region(kern_pgdir, KERNBASE,  0xffffffff - KERNBASE, 0, PTE_P | PTE_W)`
2. Note that, in 32-bit machine, the maximum address is `0xffffffff (4GB)`. The kernel uses the high memory addresses (from 3GB to 4GB).