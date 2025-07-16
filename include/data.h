#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

struct block {
  uint64_t id;
  uint32_t x;
  uint32_t y;
  uint32_t len_x;
  uint32_t len_y;
  std::vector<uint32_t> nets;
};

struct net {
  uint32_t id;
  std::vector<uint32_t> block_indices;
  std::vector<std::tuple<uint64_t, uint32_t, uint32_t>> pins;
  // TODO: Replace with single vector and keep count of beginning and end for
  // each block
  //
  // WARNING: Until we know anything better, we assume all pins are
  // at top left block corner. Pin position is neither in genlib, liberty, nor
  // verilog file
  // Coordinate system 0, 0 is top-left
};

class Data {
public:
  size_t num_blocks;
  size_t num_nets;
  uint32_t chip_x;
  uint32_t chip_y;

private:
  std::vector<block> blocks;
  std::vector<net> nets;

public:
  // Nets have to be enumerated from 0 to n and added in that order
  void add_net(net n);

  // Only call this method after all nets have been added. Assumes that b.nets
  // contains indices in this.nets
  bool add_block(block b);

  // TODO: Implement iterator
  block &get_block_by_index(size_t index);
  net &get_net_by_index(size_t index);
  // block &get_by_pos(uint32_t x, uint32_t y);
  // NOTE: Return SIZE_MAX if block is not found
  size_t get_index_from_pos(uint32_t x, uint32_t y);

  bool find_initial_placement();

  // NOTE: try_x will check if move is legal, execute if possible and update
  // pin positions in nets
  bool try_move(block &b, int32_t x, int32_t y);
  bool try_swap(block &b1, block &b2);
  bool try_rot_cw(block &b);
  bool try_rot_cc(block &b);
  bool try_flip_h(block &b);
  bool try_flip_v(block &b);

private:
  bool overlap(block &a, block &b);
  bool legal(block &a);

  struct SkylineNode {
    uint32_t x;
    uint32_t y;
    uint32_t width;
  };

  class TopLeftSkyline {
	public: 
		TopLeftSkyline(uint32_t width, uint32_t height);

		bool place(block &b);
	private:
		std::vector<SkylineNode> skyline;
		uint32_t width;
		uint32_t height;
	};
};
