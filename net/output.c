#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	while (1) {
		int r;
		envid_t from_envid;
		if ((r = ipc_recv(&from_envid, (void*)&nsipcbuf, 0)) < 0) {
			panic("recv packet fails: %e.\n", r);
		}
		if (from_envid != ns_envid) {
			continue; // ignore
		}
		struct jif_pkt* sent_pkt = (struct jif_pkt*)(&nsipcbuf);
		while ((r = sys_send_net_packet(sent_pkt->jp_data, sent_pkt->jp_len)) < 0) {
			if (r == -E_BUF_FULL) {
				// try again later
				sys_yield();
			} else {
				panic("send packet fail: %e.\n", r);
			}
		}
	}
}
