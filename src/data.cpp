#include "../include/data.h"
#include "../include/debug.h"
#include "../include/panic.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

Data::Data(uint32_t chip_x, uint32_t chip_y) : chip_x(chip_x), chip_y(chip_y) {
  // All of this is not needed
  num_blocks = 0;
  num_nets = 0;
  blocks.clear();
  nets.clear();
  reset_blocks.clear();
  reset_nets.clear();
  best_blocks.clear();
  best_nets.clear();
}

void Data::add_net(net n) {
  nets.push_back(n);
  num_nets++;
}

void Data::add_input(uint64_t id) { input_ids.push_back(id); }

void Data::add_output(uint64_t id) { output_ids.push_back(id); }

bool Data::create_pins() {
  if (input_ids.size() + output_ids.size() == 0) {
    ERROR("No pins detected")
    return true;
  }
  DEBUG("Placing ", input_ids.size(), " input pins and ", output_ids.size(),
        " output pins")

  if ((2 * input_ids.size() > chip_y) || (2 * output_ids.size() > chip_y)) {
    ERROR("Too many pins to fit on chip edge. Tried to fit ", input_ids.size(),
          " input pins and ", output_ids.size(),
          " output pin on edges of size ", chip_y, " with one space inbetween.")
    return false;
  }

  // Place input pins on left side
  uint32_t dist = chip_y / (2 * input_ids.size());
  DEBUG("Input pin dist is ", dist)
  uint32_t y = 0;

  for (auto id : input_ids) {
    DEBUG("Input pin placed at x ", 0, " y ", y)
    if (y > chip_y) {
      panic("Input pin was placed outside of chip");
    }
    auto &n = get_net_by_id(id);
    n.pins.emplace(n.pins.begin(), UINT64_MAX, 0, y);
    y += dist + 1;
  }

  // Place output pins of right side
  dist = chip_y / (2 * output_ids.size());
  DEBUG("Output pin dist is ", dist)
  y = 0;

  for (auto id : output_ids) {
    DEBUG("Output pin placed at x ", chip_x - 1, " y ", y)
    if (y > chip_y) {
      panic("Output pin was placed outside of chip");
    }
    auto &n = get_net_by_id(id);
    n.pins.emplace(n.pins.begin(), UINT64_MAX, chip_x - 1, y);
    y += dist + 1;
  }

  return true;
}

// Only call this method after all nets have been added. Assumes that b.nets
// contains ids in this.nets
void Data::add_block(block b) {
  if (b.len_x == 0 || b.len_y == 0 || b.len_x >= chip_x || b.len_y >= chip_y) {
    panic("Block has invalid size of len_x " + std::to_string(b.len_x) +
          " and len_y " + std::to_string(b.len_y) + " with chip_x " +
          std::to_string(chip_x) + " chip_y " + std::to_string(chip_y));
  }
  blocks[0].push_back(b);
  num_blocks++;
  // Add to nets
  for (uint64_t id : b.net_ids) {
    net &n = get_net_by_id(id);
    n.pins.emplace_back(b.id, b.x, b.y);
  }
}

bool Data::overlap(block &a, block &b) {
  return (a.x <= b.x + b.len_x && a.x + a.len_x >= b.x &&
          a.y <= b.y + b.len_y && a.y + a.len_y >= b.y);
}

bool Data::legal(block &a) {
  // Check if out of bounds with overflow protection
  // Because overflow is checked, it should not be needed to check if a.x and
  // a.y are already outside of the chip
  // Keep 1 unit distance to edge (Also leaves space for pins)
  if (a.x == 0 || a.y == 0 || a.x + a.len_x >= chip_x || a.x + a.len_x <= a.x ||
      a.y + a.len_y >= chip_y || a.y + a.len_y <= a.y) {
    return false;
  }
  for (block &b : blocks) {
    if (a.id == b.id) {
      continue;
    }
    if (overlap(a, b)) {
      return false;
    }
  }

  return true;
}

block &Data::get_block_by_index(size_t section, size_t index) {
  return blocks[index][0];
}

net &Data::get_net_by_index(size_t index) { return nets[index]; }

block &Data::get_block_by_id(uint64_t id) {
  for (block &b : blocks) {
    if (b.id == id) {
      return b;
    }
  }
  // Hopefully this doesn't happen
  panic("Tried to access block id " + std::to_string(id));
  return *blocks.begin();
}

net &Data::get_net_by_id(uint64_t id) {
  // Shortcut if the user was smart
  if (id < num_nets && nets[id].id == id) {
    return nets[id];
  }

  for (net &n : nets) {
    if (n.id == id) {
      return n;
    }
  }
  panic("Tried to access net id " + std::to_string(id));
  return nets[0];
}

// std::pair<size_t, size_t> Data::get_index_from_pos(uint32_t x, uint32_t y) {
//   panic("Tried to get block at position x " + std::to_string(x) + " y " +
//         std::to_string(y));
//   return {SIZE_MAX, SIZE_MAX};
// }

bool Data::find_initial_placement() {

  DEBUG("Finding initial placement")
  // Use one of the placers to find an initial legal placement
  Data::RowPacker placer(chip_x, chip_y);

  // Sort blocks by height
  DEBUG("Number of blocks ", blocks.size())
  std::sort(blocks[0].begin(), blocks[0].end(),
            [](block &a, block &b) { return a.len_y > b.len_y; });
  for (block &b : blocks) {
    DEBUG("Placing block ", b.id)
    if (!placer.place(b)) {
      // Block couldn't be placed
      ERROR("Failed to find initial placement on block with id ", b.id,
            " with len_x ", b.len_x, " and len_y ", b.len_y)
      return false;
    }
    // Update pin positions.
    // NOTE: Maybe it would be faster or more readable to do this after all
    // blocks have been placed. But to do it here makes it easier to add
    // flipping/rotations to the placer at a later point
    for (uint32_t n_id : b.net_ids) {
      net &n = get_net_by_id(n_id);
      for (size_t i = 0; i < n.pins.size(); i++) {
        auto [id, n_x, n_y] = n.pins[i];
        if (id != b.id) {
          continue;
        }
        // No over- or underflow check needed because pin must be inside or on
        // the edge of the block and it has already been checked in the
        // legal() function Pin coordinations added to new block position
        n_x += b.x;
        n_y += b.y;
        n.pins[i] = std::make_tuple(id, n_x, n_y);
      }
    }
  }

  // Sanity check!
  for (block &b : blocks) {
    if (!legal(b)) {
      // Placement is illegal
      panic("Placer produced an illegal placement");
      return false;
    }
  }
  // Found a placement
  LOG_INFO("Found an initial placement")
  return true;
}

bool Data::try_shift(block &b, int32_t x, int32_t y) {
  // NOTE: This is needed to check if a coordinate overflows and b ends up in
  // a new legal position. If this is too slow we could also just allow those
  // moves.
  if ((x < 0 && std::abs(x) > b.x) || (y < 0 && std::abs(y) > b.y)) {
    return false;
  }
  b.x += x;
  b.y += y;
  if (!legal(b)) {
    b.x -= x;
    b.y -= y;
    return false;
  }
  // Move gets executed
  // Update all pin positions in nets
  for (uint32_t n_id : b.net_ids) {
    net &n = get_net_by_id(n_id);
    for (size_t i = 0; i < n.pins.size(); i++) {
      auto [id, n_x, n_y] = n.pins[i];
      if (id != b.id) {
        continue;
      }
      // No over- or underflow check needed because pin must be inside or on
      // the edge of the block and it has already been checked in the legal()
      // function
      n_x += x;
      n_y += y;
      n.pins[i] = std::make_tuple(id, n_x, n_y);
    }
  }
  return true;
}

bool Data::try_swap(block &b1, block &b2) {
  std::swap(b1.x, b2.x);
  std::swap(b1.y, b2.y);
  if (!legal(b1) || !legal(b2)) {
    std::swap(b1.x, b2.x);
    std::swap(b1.y, b2.y);
    return false;
  }
  // Move gets executed
  // Update all pin positions in nets
  for (uint32_t n_id : b1.net_ids) {
    net &n = get_net_by_id(n_id);
    for (size_t i = 0; i < n.pins.size(); i++) {
      auto [id, n_x, n_y] = n.pins[i];
      if (id != b1.id) {
        continue;
      }
      // No over- or underflow check needed because pin must be inside or on
      // the edge of the block and it has already been checked in the legal()
      // function
      if (b1.x > b2.x) {
        n_x += b1.x - b2.x;
      } else {
        n_x -= b2.x - b1.x;
      }
      if (b1.y > b2.y) {
        n_y += b1.y - b2.y;
      } else {
        n_y -= b2.y - b1.y;
      }
      n.pins[i] = std::make_tuple(id, n_x, n_y);
    }
  }

  for (uint32_t n_id : b2.net_ids) {
    net &n = get_net_by_id(n_id);
    for (size_t i = 0; i < n.pins.size(); i++) {
      auto [id, n_x, n_y] = n.pins[i];
      if (id != b2.id) {
        continue;
      }
      // No over- or underflow check needed because pin must be inside or on
      // the edge of the block and it has already been checked in the legal()
      // function
      if (b1.x > b2.x) {
        n_x -= b1.x - b2.x;
      } else {
        n_x += b2.x - b1.x;
      }
      if (b1.y > b2.y) {
        n_y -= b1.y - b2.y;
      } else {
        n_y += b2.y - b1.y;
      }
      n.pins[i] = std::make_tuple(id, n_x, n_y);
    }
  }
  return true;
}

bool Data::try_rot_cw(block &b) {
  std::swap(b.len_x, b.len_y);
  if (!legal(b)) {
    std::swap(b.len_x, b.len_y);
    return false;
  }
  // Move gets executed
  // Update all pin positions in nets
  // Rotate pins
  for (uint32_t n_id : b.net_ids) {
    net &n = get_net_by_id(n_id);
    for (size_t i = 0; i < n.pins.size(); i++) {
      auto [id, n_x, n_y] = n.pins[i];
      if (id != b.id) {
        continue;
      }
      // No over- or underflow check needed because pin must be inside or on
      // the edge of the block and it has already been checked in the legal()
      // function
      if (n_x == b.x) {
        if (n_y == b.y) {
          n_x += b.len_x - 1;
        } else {
          n_y = b.y;
        }
      } else {
        if (n_y == b.y) {
          n_y += b.len_y - 1;
          n_x = b.x + b.len_x - 1;
        } else {
          n_x = b.x;
          n_y = b.y + b.len_y - 1;
        }
      }
      n.pins[i] = std::make_tuple(id, n_x, n_y);
    }
  }
  return true;
}

bool Data::try_rot_cc(block &b) {
  std::swap(b.len_x, b.len_y);
  if (!legal(b)) {
    std::swap(b.len_x, b.len_y);
    return false;
  }
  // Move gets executed
  // Update all pin positions in nets
  // Rotate pins
  for (uint32_t n_id : b.net_ids) {
    net &n = get_net_by_id(n_id);
    for (size_t i = 0; i < n.pins.size(); i++) {
      auto [id, n_x, n_y] = n.pins[i];
      if (id != b.id) {
        continue;
      }
      // No over- or underflow check needed because pin must be inside or on
      // the edge of the block and it has already been checked in the legal()
      // function
      if (n_x == b.x) {
        if (n_y == b.y) {
          n_y += b.len_y - 1;
        } else {
          n_y = b.y + b.len_y - 1;
          n_x += b.len_x - 1;
        }
      } else {
        if (n_y == b.y) {
          n_x = b.x;
        } else {
          n_x = b.x + b.len_x - 1;
          n_y = b.y;
        }
      }
      n.pins[i] = std::make_tuple(id, n_x, n_y);
    }
  }
  return true;
}

bool Data::try_flip_h(block &b) {
  // Move gets executed
  // Update all pin positions in nets
  // Flip pins
  for (uint32_t n_id : b.net_ids) {
    net &n = get_net_by_id(n_id);
    for (size_t i = 0; i < n.pins.size(); i++) {
      auto [id, n_x, n_y] = n.pins[i];
      if (id != b.id) {
        continue;
      }
      // No over- or underflow check needed because pin must be inside or on
      // the edge of the block and it has already been checked in the legal()
      // function
      if (n_x == b.x) {
        n_x += b.len_x - 1;
      } else {
        n_x = b.x;
      }
      n.pins[i] = std::make_tuple(id, n_x, n_y);
    }
  }
  return true;
}

bool Data::try_flip_v(block &b) {
  // Move gets executed
  // Update all pin positions in nets
  // Flip pins
  for (uint32_t n_id : b.net_ids) {
    net &n = get_net_by_id(n_id);
    for (size_t i = 0; i < n.pins.size(); i++) {
      auto [id, n_x, n_y] = n.pins[i];
      if (id != b.id) {
        continue;
      }
      // No over- or underflow check needed because pin must be inside or on
      // the edge of the block and it has already been checked in the legal()
      // function
      if (n_y == b.y) {
        n_y += b.len_y - 1;
      } else {
        n_y = b.y;
      }
      n.pins[i] = std::make_tuple(id, n_x, n_y);
    }
  }
  return true;
}

void Data::save_state() {
  reset_blocks = blocks;
  reset_nets = nets;
}

void Data::reset_state() {
  std::swap(blocks, reset_blocks);
  std::swap(nets, reset_nets);
}

void Data::save_best() {
  best_blocks = blocks;
  best_nets = nets;
}

std::vector<block> Data::get_best_blocks() {
  std::vector<block> ret;
  for (block &b : best_blocks) {
    ret.push_back(b);
  }
  return ret;
}

std::vector<net> Data::get_best_nets() { return best_nets; }
