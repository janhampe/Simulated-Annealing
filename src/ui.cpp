#include "../include/ui.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

void color_rectangle(std::vector<uint8_t> &image, uint32_t x, uint32_t len_x,
                     uint32_t y, uint32_t len_y, uint32_t line_len,
                     uint32_t units_per_pixel, uint8_t color) {
  uint32_t y_pointer = y / units_per_pixel;
  for (uint32_t i = 0; i < len_y; i += units_per_pixel, y_pointer++) {
    uint32_t x_pointer = x / units_per_pixel;
    for (uint32_t j = 0; j < len_x; j += units_per_pixel) {
      image[y_pointer * line_len + x_pointer++] = color;
    }
  }
}

void save_pgm(Data &d, const log &logger) {
  static std::vector<uint8_t> image;
  // NOTE: This might not work if chip_x % step = 0 or chip_y % step = 0
  // We'll see if this causes any problems later
  uint32_t image_width = d.chip_x / logger.units_per_pixel +
                         (d.chip_x % logger.units_per_pixel != 0);
  uint32_t image_height = d.chip_y / logger.units_per_pixel +
                          (d.chip_y % logger.units_per_pixel != 0);
  image.resize(image_width * image_height);
  std::fill(image.begin(), image.end(), VOID_COLOR);
  // Iterate through blocks and color
  for (size_t i = 0; i < d.num_blocks; i++) {
    block &b = d.get_block_by_index(i);
    color_rectangle(image, b.x, b.len_x, b.y, b.len_y, image_width,
                    logger.units_per_pixel, BLOCK_COLOR);
  }
  // Iterate through nets and color pins
  for (size_t i = 0; i < d.num_nets; i++) {
    net &n = d.get_net_by_index(i);
    for (size_t j = 0; j < n.pins.size(); j++) {
      auto [id, n_x, n_y] = n.pins[j];
      color_rectangle(image, n_x, 1, n_y, 1, image_width,
                      logger.units_per_pixel, PIN_COLOR);
    }
  }

  // Open image file
  std::string filename = logger.dir_path;
  filename.append("/")
      .append(logger.file_prefix)
      .append("_")
      .append(std::to_string(logger.step))
      .append(".pgm");
  std::ofstream file;
  file.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    ERROR("Failed to open image file")
    return;
  }
  // Write header
  file << "P5\n" << image_width << "\n" << image_height << "\n" << 255 << "\n";
  // Write image
  file.write(reinterpret_cast<const char *>(image.data()),
             image.size() * sizeof(decltype(image)::value_type));
  // Close file
  file.close();
	return;
}
