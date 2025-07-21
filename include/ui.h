#include "data.h"
#include <cstdint>
#include <string>

#define VOID_COLOR 0
#define BLOCK_COLOR 127
#define PIN_COLOR 255

struct log {
  std::string dir_path;
  std::string file_prefix;
  uint64_t step;
  uint64_t interval;
  uint32_t units_per_pixel;
};

void save_pgm(Data &d, const log &logger);
