#include "../include/data.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <tuple>
#include <utility>

Data::Data(uint32_t chip_x, uint32_t chip_y) : chip_x(chip_x), chip_y(chip_y) {
  // All of this is not needed
  num_blocks = 0;
  num_nets = 0;
  blocks.clear();
  nets.clear();
}

void Data::add_net(net n) {
  nets.push_back(n);
  num_nets++;
}

// Only call this method after all nets have been added. Assumes that b.nets
// contains ids in this.nets
void Data::add_block(block b) {
  if (b.len_x == 0 || b.len_y == 0 || b.len_x >= chip_x || b.len_y >= chip_y) {
    panic("Block has invalid size of len_x " + std::to_string(b.len_x) +
          " and len_y " + std::to_string(b.len_y) + " with chip_x " +
          std::to_string(chip_x) + " chip_y " + std::to_string(chip_y));
  }
  blocks.push_back(b);
  num_blocks++;
  // Add to nets
  for (uint64_t id : b.net_ids) {
    net &n = get_net_by_id(id);
    n.pins.emplace_back(b.id, b.x, b.y);
    // n.block_indices.push_back(blocks.size() - 1);
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
  if (a.x + a.len_x >= chip_x || a.x + a.len_x <= a.x ||
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

block &Data::get_block_by_index(size_t index) { return blocks[index]; }

net &Data::get_net_by_index(size_t index) { return nets[index]; }

block &Data::get_block_by_id(uint64_t id) {
  // std::cout << "Searching for block id " << std::to_string(id) << std::endl;
  for (block &b : blocks) {
    // std::cout << "Id " << b.id << std::endl;
    if (b.id == id) {
      return b;
    }
  }
  // Hopefully this doesn't happen
  panic("Tried to access block id " + std::to_string(id));
  return blocks[0];
}

net &Data::get_net_by_id(uint64_t id) {
  for (net &n : nets) {
    if (n.id == id) {
      return n;
    }
  }
  panic("Tried to access net id " + std::to_string(id));
  return nets[0];
}

size_t Data::get_index_from_pos(uint32_t x, uint32_t y) {
  for (size_t i = 0; i < num_blocks; i++) {
    if (get_block_by_index(i).x == x && get_block_by_index(i).y == y) {
      return i;
    }
  }
  panic("Tried to get block at position x " + std::to_string(x) + " y " +
        std::to_string(y));
  return SIZE_MAX;
}

bool Data::find_initial_placement() {

  // Use one of the placers to find an initial legal placement
  Data::RowPacker placer(chip_x, chip_y);

  // Sort blocks by height
  std::sort(blocks.begin(), blocks.end(),
            [](block &a, block &b) { return a.len_y >= b.len_y; });
  for (block &b : blocks) {
    if (!placer.place(b)) {
      // Block couldn't be placed
      std::cout << "ERROR: Failed to find initial placement on block with id "
                << b.id << " with len_x " << b.len_x << " and len_y " << b.len_y
                << std::endl;
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
        // the edge of the block and it has already been checked in the legal()
        // function
        n_x = b.x;
        n_y = b.y;
        n.pins[i] = std::make_tuple(id, n_x, n_y);
      }
    }
  }

  // Sanity check!
  for (block &b : blocks) {
    if (!legal(b)) {
      // Placement is illegal
      std::cout << "ERROR: placer produced an illegal placement" << std::endl;
      return false;
    }
  }
  // Found a placement
  std::cout << "INFO: Found an initial placement" << std::endl;
  return true;
}

bool Data::try_move(block &b, int32_t x, int32_t y) {
  // NOTE: This is needed to check if a coordinate overflows and b ends up in a
  // new legal position. If this is too slow we could also just allow those
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
          n_x += b.len_x;
        } else {
          n_y = b.y;
        }
      } else {
        if (n_y == b.y) {
          n_y += b.len_y;
          n_x = b.x + b.len_x;
        } else {
          n_x = b.x;
          n_y = b.y + b.len_y;
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
          n_y += b.len_y;
        } else {
          n_y = b.y + b.len_y;
          n_x += b.len_x;
        }
      } else {
        if (n_y == b.y) {
          n_x = b.x;
        } else {
          n_x = b.x + b.len_x;
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
        n_x += b.len_x;
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
        n_y += b.len_y;
      } else {
        n_y = b.y;
      }
      n.pins[i] = std::make_tuple(id, n_x, n_y);
    }
  }
  return true;
}
