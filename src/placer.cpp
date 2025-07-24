#include "../include/data.h"

// NOTE: TopLeftSkyline not implemented for now
Data::TopLeftSkyline::TopLeftSkyline(uint32_t width, uint32_t height)
    : width(width), height(height) {
  skyline.push_back({0, 0, width});
}

bool Data::TopLeftSkyline::place(block &b) { return true; }

Data::RowPacker::RowPacker(uint32_t width, uint32_t height)
    : width(width), height(height), current_width(width), current_height(1), next_height(1)
      {}

bool Data::RowPacker::place(block &b) {
  // Check if block fits on the chip at all
  // Obviously this doesn't allow the block to be rotated
  if (b.len_x + 1 >= width || b.len_y + current_height >= height) {
    return false;
  }

  // Check if block still fits on this row
  if (current_width + b.len_x >= width) {
    // Start new row
    current_height = next_height;
    next_height += b.len_y + 1;
    current_width = 1;
  }
  if (current_height + b.len_y >= height) {
    // No space left to fit this block after moving a row down
    return false;
  }
  // Found a space
  b.x = current_width;
  b.y = current_height;
  current_width += b.len_x + 1;

  return true;
}
