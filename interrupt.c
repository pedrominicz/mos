#include "interrupt.h"
#include "terminal.h"
#include "x86.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Interrupt {
  // `pusha`
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  // `push gs`
  uint16_t gs;
  uint16_t gs_padding;
  // `push fs`
  uint16_t fs;
  uint16_t fs_padding;
  // `push es`
  uint16_t es;
  uint16_t es_padding;
  // `push ds`
  uint16_t ds;
  uint16_t ds_padding;
  // `push dword %1`
  uint32_t number;
  // `push dword 0` or pushed by the CPU.
  uint32_t error_code;
  // Pushed by the CPU.
  uint32_t eip;
  uint16_t cs;
  uint16_t cs_padding;
  uint16_t eflags;
} Interrupt;

extern uintptr_t interrupts[256]; // Defined in "interrupt-stubs.asm".
static uint64_t idt[256];

// Initialize programmable interrupt controller (PIC).
static inline void init_pic(void) {
  // Restart PICs.
  outb(0x20, 0x11); // Master PIC command port.
  outb(0xa0, 0x11); // Slave PIC command port.
  // Configure interrupt vector offset.
  outb(0x21, 0x20); // Master PIC data port.
  outb(0xa1, 0x28); // Slave PIC data port.
  // Connect both PICs.
  outb(0x21, 0x04); // Tell master slave is at interrupt request line 2.
  outb(0xa1, 0x02); // Tell slave its cascade identity.
  // XXX
  outb(0x21, 0x01);
  outb(0xa1, 0x01);
  // Set interrupt masks. Ignore everything except the programmable interval
  // timer (PIT).
  outb(0x21, 0b11111110);
  outb(0xa1, 0b11111111);
}

static inline uint64_t make_interrupt_gate(const uintptr_t interrupt) {
  const uint64_t interrupt_low = interrupt;
  const uint64_t interrupt_high = interrupt >> 16;

  uint64_t idt_entry = 0;
  idt_entry |= interrupt_low;
  idt_entry |= interrupt_high << 48;
  idt_entry |= 0x0008 << 16; // Kernel code selector.
  // Interrupt type and attributes.
  idt_entry |= (uint64_t)0b10001110 << 40;
  return idt_entry;
}

static inline void init_idt(void) {
  for(size_t i = 0; interrupts[i]; ++i) {
    idt[i] = make_interrupt_gate(interrupts[i]);
  }

  volatile uint64_t idt_descriptor = 0;
  idt_descriptor |= sizeof(idt) - 1;
  idt_descriptor |= (uint64_t)(uint32_t)idt << 16;

  asm volatile ("lidt [%0]" :: "r"(&idt_descriptor));
}

void init_interrupt(void) {
  init_pic();
  init_idt();
}

void interrupt_handle(const Interrupt* const interrupt) {
  // The first interrupt will be a divide by zero on `init_mios`. Just rewrite
  // the `div ebx` instruction with NOPs.
  static bool first_time = true;
  if(first_time) {
    *(uint16_t*)interrupt->eip = 0x9090;
    first_time = false;
  } else {
    terminal_putchar('.');
  }
}