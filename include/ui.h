#include "data.h"
#include <string>

#define VOID_COLOR 0
#define BLOCK_COLOR 127
#define PIN_COLOR 255

void save_pgm(Data &d, std::string dir_path, std::string image_file_prefix,
              uint64_t step, uint32_t units_per_pixel);
