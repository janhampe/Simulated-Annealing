#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

// NOTE: Block ids and Net ids have to be unique

struct block {
  uint64_t id;
  uint32_t x;
  uint32_t y;
  uint32_t len_x;
  uint32_t len_y;
  std::vector<uint64_t> net_ids;
};

struct net {
  uint64_t id;
  std::vector<std::tuple<uint64_t, uint32_t, uint32_t>> pins;
  // TODO: Replace with single vector and keep count of beginning and end for
  // each block
  //
  // NOTE: Until we know anything better, we assume all pins are
  // at top left block corner. Pin position is neither in genlib, liberty, nor
  // verilog file
  // Coordinate system 0, 0 is top-left
};

class Data {
private:
  class BlockList {
  public:
    struct Iterator {
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type = block;
      using element_type = block;
      using pointer = block *;
      using reference = block &;
      using difference_type = std::ptrdiff_t;

      Iterator(BlockList *ptr, size_t outer, size_t inner)
          : list(ptr), outer(outer), inner(inner) {}

      block &operator*() const { return list->blocks[outer][inner]; }

      block *operator->() const { return &operator*(); }

      Iterator &operator++() {
        inner++;
        while (inner >= list->blocks[outer].size()) {
          inner = 0;
          outer++;
        }
        return *this;
      }

      Iterator &operator--() {
        if (inner > 0) {
          inner--;
          return *this;
        } else {
          while (list->blocks[outer].size() == 0 && outer > 0) {
            outer--;
          }
          if (list->blocks[outer].size() == 0) {
            inner = 0;
            return *this;
          }
          inner = list->blocks[outer].size() - 1;
          return *this;
        }
      }

      bool operator==(const Iterator &b) {
        return list == b.list && outer == b.outer && inner == b.inner;
      }

      bool operator!=(const Iterator &b) {
        return list != b.list || outer != b.outer || inner != b.inner;
      }

    private:
      BlockList *list;
      size_t outer;
      size_t inner;
    };

    Iterator begin() { return Iterator(this, 0, 0); }
    Iterator end() {
      return Iterator(this, blocks.size() - 1, blocks[0].size());
    }

    BlockList &operator=(const BlockList &b) {
      blocks = b.blocks;
      return *this;
    }

    std::vector<block> &operator[](size_t index) { return blocks[index]; }

    void clear() { blocks.clear(); }

    // block &operator[](size_t i) {
    // 	size_t index = 0;
    // }

  private:
    std::vector<std::vector<block>> blocks;
    // TODO: Test if an extra vector with each blocks[] subvector's sizes for
    // index computation makes any improvements
  };

public:
  size_t num_blocks;
  size_t num_nets;
  uint32_t chip_x;
  uint32_t chip_y;

private:
  BlockList blocks;
  std::vector<net> nets;
  uint32_t largest_block;
  uint32_t sec_length;
  uint32_t sec_per_line;

  std::vector<uint64_t> input_ids;
  std::vector<uint64_t> output_ids;

  BlockList reset_blocks;
  std::vector<net> reset_nets;

  BlockList best_blocks;
  std::vector<net> best_nets;

public:
  Data(uint32_t chip_x, uint32_t chip_y);
  // Nets should be enumerated from id 0 to n and added in that order to make
  // the program run much faster
  void add_net(net n);

  void add_input(uint64_t id);
  void add_output(uint64_t id);

  bool create_pins();

  // Only call this method after all nets have been added. Assumes that b.nets
  // contains net ids in this.net_ids
  void add_block(block b);

  void make_sections();
  void update_sections();

  // TODO: Implement iterator (Is it needed tho?)
  block &get_block_by_index(size_t section, size_t index);
  net &get_net_by_index(size_t index);
  block &get_block_by_id(uint64_t id);
  net &get_net_by_id(uint64_t id);

  // block &get_by_pos(uint32_t x, uint32_t y);
  // std::pair<size_t, size_t> get_index_from_pos(uint32_t x, uint32_t y);

  // After all blocks have been added call this to find an initial placement
  bool find_initial_placement();

  // NOTE: overlap and legal are only public to allow for testing
  bool overlap(block &a, block &b);
  bool legal(block &a);

  // try_x will check if move is legal, execute if possible and update
  // pin positions in nets
  bool try_shift(block &b, int32_t x, int32_t y);
  bool try_swap(block &b1, block &b2);
  bool try_rot_cw(block &b);
  bool try_rot_cc(block &b);
  bool try_flip_h(block &b);
  bool try_flip_v(block &b);

  // WARNING: These copy the entire blocks and nets vectors, so they can get
  // very expensive!
  void save_state();
  void reset_state();

  void save_best();

  std::vector<block> get_best_blocks();
  std::vector<net> get_best_nets();

private:
  struct SkylineNode {
    uint32_t x;
    uint32_t y;
    uint32_t width;
  };

  // NOTE: Leave this until we see it is actually needed. There should be more
  // than enough space on the chip usually, that super efficient packing is not
  // required.
  class TopLeftSkyline {
  public:
    TopLeftSkyline(uint32_t width, uint32_t height);

    bool place(block &b);

  private:
    std::vector<SkylineNode> skyline;
    uint32_t width;
    uint32_t height;
  };

  class RowPacker {
  public:
    RowPacker(uint32_t width, uint32_t height);

    // Will place the blocks in rows with the first block's height determining
    // the height of the next row Assumes that placed blocks are sorted by heigt
    // in descending order Blocks are blaced one unit apart from the edge and
    // each other
    bool place(block &b);

  private:
    uint32_t width;
    uint32_t height;
    uint32_t current_width;
    uint32_t current_height;
    uint32_t next_height;
  };
};
