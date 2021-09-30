// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800
extern volatile pde_t uvpd[];
extern volatile pte_t uvpt[];
//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if (!(err & FEC_WR)) {
		panic("page fault at: %x! Not caused by write\n", addr);
	}
	pte_t* pgt_entry = (pte_t*)uvpt + PGNUM(addr);
	if (!(*pgt_entry & PTE_COW)) {
		// not copy-on-write
		panic("page fault at: %x!\n", addr);
	}
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	int perm = PTE_U | PTE_W | PTE_P;
	void* pg_addr = (void*)(PGNUM(addr) * PGSIZE);
	if ((r = sys_page_alloc(0, PFTEMP, perm)) < 0) {
		panic("pgfault: alloc new page fails: %d.\n", r);
	}
	memcpy(PFTEMP, pg_addr, PGSIZE);
	if ((r = sys_page_map(0, PFTEMP, 0, pg_addr, perm)) < 0) {
		panic("pgfault: map new page fails: %d.\n", r);
	}
	if ((r = sys_page_unmap(0, PFTEMP)) < 0) {
		panic("pgfault: unmap PFTEMP fails: %d.\n", r);
	}
	// cprintf("pgfault handler: finished handling pgfault at %x\n", pg_addr);
	// panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	// panic("duppage not implemented");
	// cprintf("\n--------------------\nbegin dup: %x\n", pn * PGSIZE);
	r = sys_page_map(0, (void*)(pn*PGSIZE), envid, (void*)(pn*PGSIZE), PTE_COW | PTE_U | PTE_P);
	if (r < 0) {
		panic("duplicating page fails.\n");
	}
	// cprintf("set up COW: %x for child, r: %x\n", pn, &r);
	sys_page_map(0, (void*)(pn*PGSIZE), 0, (void*)(pn*PGSIZE), PTE_COW | PTE_U | PTE_P);
	// cprintf("dup finished\n--------------------\n");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	// panic("fork not implemented");
	set_pgfault_handler(pgfault);
	envid_t pid = sys_exofork();
	if (pid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())]; // reset the thisenv
		return 0;
	}
	if (pid < 0) {
		return pid;
	}
	// parent
	for (uint32_t cur = 0; cur != UTOP - PGSIZE; cur += PGSIZE) {
		int offset = PGNUM(cur);
		pte_t* pgt_entry = (pte_t*)uvpt + offset;
		pde_t* pdt_entry = (pde_t*)uvpd + PDX(cur);
		if (*pdt_entry && *pgt_entry) {
			// page exists
			if (((*pgt_entry) & PTE_W) || ((*pgt_entry) & PTE_COW)) {
				duppage(pid, offset);
			} else if ((*pgt_entry) & PTE_U) {
				// readable
				// cprintf("map readable address: %x\n", cur);
				sys_page_map(0, (void*)cur, pid, (void*)cur, PTE_P | PTE_U);
			}
		}
	}
	// exception stack
	sys_page_alloc(pid, (void*)UXSTACKTOP - PGSIZE, PTE_U | PTE_P | PTE_W);
	extern void _pgfault_upcall(void);
	sys_env_set_pgfault_upcall(pid, _pgfault_upcall);
	sys_env_set_status(pid, ENV_RUNNABLE);
	// cprintf("setup child!\n");
	return pid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
