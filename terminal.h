#ifndef MIOS_TERMINAL_H
#define MIOS_TERMINAL_H

#include <stdint.h>

void init_terminal(void);

void terminal_clear(void);
void terminal_print_hex(uintmax_t n);
void terminal_putchar(const char c);

static inline void terminal_print(const char* s) {
  for(char c = *s; c != 0; c = *(++s)) {
    terminal_putchar(c);
  }
}

#endif // MIOS_TERMINAL_H
