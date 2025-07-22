/*  Written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include "../include/xoshiro256pp.h"
#include <iostream>

static uint64_t xo_s[4];

void xo_init_state(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
  xo_s[0] = a;
  xo_s[1] = b;
  xo_s[2] = c;
  xo_s[3] = d;
}

static inline uint64_t xo_rotl(const uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}

uint64_t xo_next(void) {
  // DEBUG("xo_s is ", xo_s[0], " ", xo_s[1], " ", xo_s[2], " ", xo_s[3])
  const uint64_t result = xo_rotl(xo_s[0] + xo_s[3], 23) + xo_s[0];

  const uint64_t t = xo_s[1] << 17;

  xo_s[2] ^= xo_s[0];
  xo_s[3] ^= xo_s[1];
  xo_s[1] ^= xo_s[2];
  xo_s[0] ^= xo_s[3];

  xo_s[2] ^= t;

  xo_s[3] = xo_rotl(xo_s[3], 45);

  return result;
}

/* This is the jump function for the generator. It is equivalent
   to 2^128 calls to next(); it can be used to generate 2^128
   non-overlapping subsequences for parallel computations. */

void xo_jump(void) {
  static const uint64_t JUMP[] = {0x180ec6d33cfd0aba, 0xd5a61266f0c9392c,
                                  0xa9582618e03fc9aa, 0x39abdc4529b1661c};

  uint64_t s0 = 0;
  uint64_t s1 = 0;
  uint64_t s2 = 0;
  uint64_t s3 = 0;
  for (size_t i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
    for (int b = 0; b < 64; b++) {
      if (JUMP[i] & UINT64_C(1) << b) {
        s0 ^= xo_s[0];
        s1 ^= xo_s[1];
        s2 ^= xo_s[2];
        s3 ^= xo_s[3];
      }
      xo_next();
    }

  xo_s[0] = s0;
  xo_s[1] = s1;
  xo_s[2] = s2;
  xo_s[3] = s3;
}

/* This is the long-jump function for the generator. It is equivalent to
   2^192 calls to next(); it can be used to generate 2^64 starting points,
   from each of which jump() will generate 2^64 non-overlapping
   subsequences for parallel distributed computations. */

void xo_long_jump(void) {
  static const uint64_t LONG_JUMP[] = {0x76e15d3efefdcbbf, 0xc5004e441c522fb3,
                                       0x77710069854ee241, 0x39109bb02acbe635};

  uint64_t s0 = 0;
  uint64_t s1 = 0;
  uint64_t s2 = 0;
  uint64_t s3 = 0;
  for (size_t i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
    for (int b = 0; b < 64; b++) {
      if (LONG_JUMP[i] & UINT64_C(1) << b) {
        s0 ^= xo_s[0];
        s1 ^= xo_s[1];
        s2 ^= xo_s[2];
        s3 ^= xo_s[3];
      }
      xo_next();
    }

  xo_s[0] = s0;
  xo_s[1] = s1;
  xo_s[2] = s2;
  xo_s[3] = s3;
}
