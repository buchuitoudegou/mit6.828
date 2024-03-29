#include "kern/e1000.h"
#include "kern/pmap.h"
#include "inc/string.h"
#include "inc/error.h"

// LAB 6: Your driver code here

volatile uint32_t* e1000_reg_space;
volatile struct tx_desc tx_buffer[E1000_TX_BUFFER_LEN] = {0};
volatile struct rx_desc rx_buffer[E1000_RX_BUFFER_LEN] = {0};
uint8_t mac_addr[6] = { 0x52, 0x54, 0x0, 0x12, 0x34, 0x56 }; // hard-coded
uint8_t tx_buffer_payload[E1000_TX_PKT_SIZE * E1000_TX_BUFFER_LEN]; // 1518 byte for each descriptor
uint8_t rx_buffer_payload[E1000_RX_PKT_SIZE * E1000_RX_BUFFER_LEN];
uint8_t tail_offset = 0;

int e1000_init() {
  // setup tx register
  uint32_t tx_buffer_pa = (uint32_t)PADDR((void*)tx_buffer);
  uint32_t rx_buffer_pa = (uint32_t)PADDR((void*)rx_buffer);
  e1000_reg_space[OFFSET(E1000_TDBAH)] = 0;
  e1000_reg_space[OFFSET(E1000_TDBAL)] = tx_buffer_pa; // base address of the queue
  e1000_reg_space[OFFSET(E1000_TDH)] = 0; // head offset
  e1000_reg_space[OFFSET(E1000_TDT)] = 0; // tail offset
  e1000_reg_space[OFFSET(E1000_TDLEN)] = sizeof(struct tx_desc) * E1000_TX_BUFFER_LEN; // length of the queue (in byte)
  e1000_reg_space[OFFSET(E1000_TIPG)] = (6 << 20) | (8 << 10) | 10;
  e1000_reg_space[OFFSET(E1000_TCTL)] = (1 << 1) | (1 << 3) | (0x10 << 4) | (0x40 << 12);
  // setup rx register
  e1000_reg_space[OFFSET(E1000_RA)] = 
    mac_addr[0] | (mac_addr[1] << 8) | (mac_addr[2] << 16) | (mac_addr[3] << 24);
  e1000_reg_space[OFFSET(E1000_RA) + 1] = mac_addr[4] | (mac_addr[5] << 8) | (0x1 << 31);
  e1000_reg_space[OFFSET(E1000_MTA)] = 0;
  e1000_reg_space[OFFSET(E1000_RDBAL)] = rx_buffer_pa;
  e1000_reg_space[OFFSET(E1000_RDBAH)] = 0;
  e1000_reg_space[OFFSET(E1000_RDLEN)] = sizeof(struct rx_desc) * E1000_RX_BUFFER_LEN;
  e1000_reg_space[OFFSET(E1000_RDH)] = 0;
  e1000_reg_space[OFFSET(E1000_RDT)] = E1000_RX_BUFFER_LEN - 1;
  e1000_reg_space[OFFSET(E1000_RCTL)] = (0x1 << 1) | (0x1 << 5) | (0x1 << 26);

  for (int i = 0; i < E1000_RX_BUFFER_LEN; ++i) {
    rx_buffer[i].buffer_addr = (uint32_t)PADDR((void*)&rx_buffer_payload[i*E1000_RX_PKT_SIZE]);
    rx_buffer[i].status = 0; // clear the DD bit
  }
  return 0;
}

int e1000_trans_packet(const void* src, int len) {
  if (len > E1000_TX_PKT_SIZE) {
    return -E_INVAL;
  }
  // find the free slot
  int trv_cnt = 0;
  while (trv_cnt < E1000_TX_BUFFER_LEN) {
    bool rs = tx_buffer[tail_offset].cmd & 0x8;
    bool dd = tx_buffer[tail_offset].status & 0x1;
    if (!rs || dd) {
      // rs bit not set: the slot hasn't been used
      // rs is set: the slot has been used previously
      //     - dd is set: the packet of the slot has been sent by the card
      int payload_idx = tail_offset * E1000_TX_PKT_SIZE;
      memcpy(tx_buffer_payload + payload_idx, src, len);
      tx_buffer[tail_offset].addr = PADDR((void*)(tx_buffer_payload + payload_idx));
      tx_buffer[tail_offset].length = len;
      tx_buffer[tail_offset].cmd = (1 << 3) | 1; // set the RS bit and the end-of-packet bit
      tx_buffer[tail_offset].status = 0; // clear the DD bit
      e1000_reg_space[OFFSET(E1000_TDT)] = (tail_offset + 1) % E1000_TX_BUFFER_LEN; // inform the card
      tail_offset = (tail_offset + 1) % E1000_TX_BUFFER_LEN;
      return 0; // successfully sent
    } else {
      tail_offset = (tail_offset + 1) % E1000_TX_BUFFER_LEN;
      trv_cnt ++;
    }
  }
  // the buffer is full
  // the caller handles the re-send
  return -E_BUF_FULL;
}

int e1000_recv_packet(void* dst, int* len) {
  int nxt = (e1000_reg_space[OFFSET(E1000_RDT)] + 1) % E1000_RX_BUFFER_LEN;
  if (rx_buffer[nxt].status & 0x1) {
    // DD bit is set
    memcpy(dst, (void*)KADDR(rx_buffer[nxt].buffer_addr), rx_buffer[nxt].length);
    *len = rx_buffer[nxt].length;
    rx_buffer[nxt].status = 0; // clear the DD bit
    e1000_reg_space[OFFSET(E1000_RDT)] = nxt;
    return 0;
  }
  // buffer empty
  return -E_BUF_EMPTY;
}

int e1000_boot(struct pci_func *f) {
  pci_func_enable(f);
  e1000_reg_space = mmio_map_region(f->reg_base[0], f->reg_size[0]);
  e1000_init();
  return 0;
}
