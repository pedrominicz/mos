#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

static uint64_t idt[256];

static inline void hang(void) {
  while(1) {
    asm volatile ("cli");
    asm volatile ("hlt");
  }
}

static void test_interrupt_handler(void) {
  terminal_clear();
  terminal_print("(:\n");
}

static inline uint64_t make_interrupt_gate(uintptr_t interrupt) {
  const uint64_t interrupt_low = interrupt;
  const uint64_t interrupt_high = interrupt >> 16;

  uint64_t idt_entry = 0;
  idt_entry |= interrupt_low;
  idt_entry |= interrupt_high << 48;
  idt_entry |= 0x0008 << 16;
  // Interrupt type and attributes.
  idt_entry |= (uint64_t)0b10001110 << 40;
  return idt_entry;
}

void mios_init(void) {
  idt[0] = make_interrupt_gate((uintptr_t)test_interrupt_handler);

  uint64_t idt_descriptor = 0;
  idt_descriptor |= sizeof(idt) - 1;
  idt_descriptor |= (uint64_t)(uint32_t)idt << 16;
  asm volatile("nop");

  asm volatile ("lidt [%0]" :: "r"(&idt_descriptor));

  terminal_clear();
  asm volatile ("int 0x00");
  hang();
}
