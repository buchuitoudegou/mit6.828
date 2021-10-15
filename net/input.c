#include "ns.h"

extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
	int r, len, perm = PTE_U | PTE_P | PTE_W;
	struct jif_pkt* recv_page = (struct jif_pkt*)&nsipcbuf;
	while (1) {
		if ((r = sys_page_alloc(0, (void*)recv_page, perm)) < 0) {
			panic("input: alloc new page fails: %e.\n", r);
		}
		while ((r = sys_recv_net_packet((void*)recv_page->jp_data, &recv_page->jp_len)) < 0) {
			if (r == -E_BUF_EMPTY) {
				// try again later
				sys_yield();
			} else {
				panic("input: recv packet fails: %e.\n", r);
			}
		}
		ipc_send(ns_envid, NSREQ_INPUT, (void*)recv_page, perm);
		if ((r = sys_page_unmap(0, (void*)recv_page)) < 0) {
			panic("input: free page error: %e.\n", r);
		}
	}
}
