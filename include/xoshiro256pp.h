#include <cstddef>
#include <cstdint>
#include <stdint.h>

void xo_init_state(uint64_t a, uint64_t b, uint64_t c, uint64_t d);

uint64_t xo_next(void);

void xo_jump(void);

void xo_long_jump(void);
