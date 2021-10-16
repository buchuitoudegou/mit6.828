# lab6

## overview
1. The Ethernet card can only communicate with the kernel. When initializing the kernel, the `pci_init` function scans the PCI bus and identifies the designated hardware. After enabling the device, the kernel allocates a region of memory to read/write the registers of the device (aka. MMIO).
2. The Ethernet card can directly access the memory without the CPU involved (aka. DMA).
3. Two circular queues as the receive buffer and the transmit buffer are allocated in the kernel space. In this lab, the queues are static arrays of descriptors. Each descriptor describes where the data is stored, how long it is, etc. There are 4 registers maintaining the heads and tails of them.
4. Conventionally, the Ethernet card can also raise an interrupt to inform the CPU of the comming data. In this lab, we receive/transmit the data via polling.
5. The network module is an independent process from the kernel, which implements some general network interface, the network stack (e.g. TCP/IP, ARP, etc), and so on. It pulls/pushes data from/to the kernel's Ethernet card driver via system call. Other user processes invokes the network interfaces using IPC (aka. syscall), which is similar to the filesystem.

## Exercise 3-8
In these sections, we are required to enable the transmit function of the Ethernet card. Basically, just follow the instructions of chapter 14.4 in the manual. Note that, the Ethernet card doesn't access the memory via MMU and thus, the physical address of the queue should be set to the register.

When implementing the transmitting syscall, accessing the head register is not allowed since it is not reliable referred to the manual. Instead, checking the `DD` bit of the descriptor is recommeneded to check if the descriptor is sent.

## Exercise 9-12
Setting the receive function is similar. In addition to this, the MAC address should also be set to filter the packets. In this lab, the MAC address is hard-coded and the registers store the address from low bit to high bit. Further, the tail should point to the last descriptor that is free. When the head is equal to the tail, the card will stopped writting since there is no available space.

## Exercise 10
Implement the http web server. The network module regards the sockets as files. Simply `write` data to the socket's `fd` is plausible.