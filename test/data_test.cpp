#include "../include/data.h"
#include "doctest.h"
#include <cstdint>

// Data Tests

// Test Add Net and Block
TEST_CASE("Add Net and Block") {
  Data data(20, 20);
  data.add_net({0, {}});
  data.add_net({2, {}});
  data.add_net({3, {}});
  REQUIRE_EQ(data.num_nets, 3);

  data.add_block({10, 0, 0, 2, 3, {0, 3}});
  data.add_block({11, 5, 0, 2, 2, {0}});
  data.add_block({12, 4, 2, 1, 1, {0, 2}});
  data.add_block({13, 9, 9, 2, 3, {2, 3}});
  REQUIRE_EQ(data.num_blocks, 4);

  SUBCASE("Pins added correctly") {
    uint64_t id;
    uint32_t x;
    uint32_t y;
    {
      net &n = data.get_net_by_id(0);
      {
        std::tie(id, x, y) = n.pins[0];
        block &b = data.get_block_by_id(10);
        CHECK_EQ(id, 10);
        CHECK_EQ(x, b.x);
        CHECK_EQ(y, b.y);
      }
      {
        std::tie(id, x, y) = n.pins[1];
        block &b = data.get_block_by_id(11);
        CHECK_EQ(id, 11);
        CHECK_EQ(x, b.x);
        CHECK_EQ(y, b.y);
      }

      {
        std::tie(id, x, y) = n.pins[2];
        block &b = data.get_block_by_id(12);
        CHECK_EQ(id, 12);
        CHECK_EQ(x, b.x);
        CHECK_EQ(y, b.y);
      }
    }
    {
      net &n = data.get_net_by_id(2);
      {
        std::tie(id, x, y) = n.pins[0];
        block &b = data.get_block_by_id(12);
        CHECK_EQ(id, 12);
        CHECK_EQ(x, b.x);
        CHECK_EQ(y, b.y);
      }
      {
        std::tie(id, x, y) = n.pins[1];
        block &b = data.get_block_by_id(13);
        CHECK_EQ(id, 13);
        CHECK_EQ(x, b.x);
        CHECK_EQ(y, b.y);
      }
    }

    {
      net &n = data.get_net_by_id(3);
      {
        std::tie(id, x, y) = n.pins[0];
        block &b = data.get_block_by_id(10);
        CHECK_EQ(id, 10);
        CHECK_EQ(x, b.x);
        CHECK_EQ(y, b.y);
      }
      {
        std::tie(id, x, y) = n.pins[1];
        block &b = data.get_block_by_id(13);
        CHECK_EQ(id, 13);
        CHECK_EQ(x, b.x);
        CHECK_EQ(y, b.y);
      }
    }
  }
}

// Test overlap
TEST_CASE("Test overlap()") {
  Data data(20, 20);

  // Misses left
  SUBCASE("Miss left") {
    block a = {0, 0, 0, 1, 2, {}};
    block b = {1, 3, 0, 2, 2, {}};
    CHECK_FALSE(data.overlap(a, b));
  }
  // Miss right
  SUBCASE("Miss right") {
    block a = {0, 3, 0, 2, 2, {}};
    block b = {1, 0, 0, 1, 2, {}};
    CHECK_FALSE(data.overlap(a, b));
  }
  // Miss top
  SUBCASE("Miss top") {
    block a = {0, 0, 0, 2, 2, {}};
    block b = {1, 0, 3, 1, 2, {}};
    CHECK_FALSE(data.overlap(a, b));
  }
  // Miss bottom
  SUBCASE("Miss bottom") {
    block a = {0, 1, 4, 2, 2, {}};
    block b = {1, 0, 0, 1, 2, {}};
    CHECK_FALSE(data.overlap(a, b));
  }
  // Touch left
  SUBCASE("Touch left") {
    block a = {0, 1, 0, 2, 2, {}};
    block b = {1, 3, 0, 1, 2, {}};
    CHECK(data.overlap(a, b));
  }
  // Touch right
  SUBCASE("Touch right") {
    block a = {0, 2, 0, 2, 2, {}};
    block b = {1, 1, 0, 1, 2, {}};
    CHECK(data.overlap(a, b));
  }
  // Touch top
  SUBCASE("Touch top") {
    block a = {0, 3, 0, 2, 2, {}};
    block b = {1, 2, 2, 1, 2, {}};
    CHECK(data.overlap(a, b));
  }
  // Touch bottom
  SUBCASE("Touch bottom") {
    block a = {0, 3, 2, 2, 2, {}};
    block b = {1, 3, 0, 1, 2, {}};
    CHECK(data.overlap(a, b));
  }
  // Cut left
  SUBCASE("Cut left") {
    block a = {0, 2, 0, 5, 2, {}};
    block b = {1, 4, 0, 4, 2, {}};
    CHECK(data.overlap(a, b));
  }
  // Cut right
  SUBCASE("Cut right") {
    block a = {0, 2, 0, 4, 2, {}};
    block b = {1, 0, 0, 4, 2, {}};
    CHECK(data.overlap(a, b));
  }
  // Cut bottom
  SUBCASE("Cut bottom") {
    block a = {0, 2, 1, 2, 2, {}};
    block b = {1, 2, 0, 1, 2, {}};
    CHECK(data.overlap(a, b));
  }
  // Cut top
  SUBCASE("Cut top") {
    block a = {0, 1, 0, 2, 2, {}};
    block b = {1, 1, 1, 1, 2, {}};
    CHECK(data.overlap(a, b));
  }
  // Cut across horizontally
  SUBCASE("Cut across horizontally") {
    block a = {0, 1, 2, 6, 2, {}};
    block b = {1, 2, 0, 1, 5, {}};
    CHECK(data.overlap(a, b));
  }
  // Cut across vertically
  SUBCASE("Cut across vertically") {
    block a = {0, 2, 0, 1, 7, {}};
    block b = {1, 0, 3, 5, 2, {}};
    CHECK(data.overlap(a, b));
  }
  // Inside
  SUBCASE("Inside") {
    block a = {0, 3, 2, 2, 2, {}};
    block b = {1, 1, 1, 6, 6, {}};
    CHECK(data.overlap(a, b));
  }
  // On top of each other
  SUBCASE("On top of each other") {
    block a = {0, 2, 2, 2, 4, {}};
    block b = {1, 2, 2, 2, 4, {}};
    CHECK(data.overlap(a, b));
  }
}

// Test legal()
TEST_CASE("Test legal()") {
  Data data(20, 20);

  data.add_net({0, {}});
  data.add_net({2, {}});
  data.add_net({3, {}});
  REQUIRE_EQ(data.num_nets, 3);

  data.add_block({10, 0, 0, 2, 3, {0, 3}});
  data.add_block({11, 5, 0, 2, 2, {0}});
  data.add_block({12, 4, 2, 1, 1, {0, 2}});
  data.add_block({13, 9, 9, 2, 3, {2, 3}});
  REQUIRE_EQ(data.num_blocks, 4);

  SUBCASE("Single legal block") {
    block b = {0, 15, 15, 1, 2, {}};
    CHECK(data.legal(b));
  }

  SUBCASE("Block with overlap") {
    block b = {0, 1, 1, 1, 2, {}};
    CHECK_FALSE(data.legal(b));
  }

  SUBCASE("Block completely outside chip x") {
    block b = {0, 23, 3, 1, 1, {}};
    CHECK_FALSE(data.legal(b));
  }

  SUBCASE("Block completely outside chip y") {
    block b = {0, 3, 30, 1, 1, {}};
    CHECK_FALSE(data.legal(b));
  }

  SUBCASE("Block reaching outside chip x") {
    block b = {0, 3, 13, 25, 1, {}};
    CHECK_FALSE(data.legal(b));
  }

  SUBCASE("Block reaching outside chip y") {
    block b = {0, 3, 13, 2, 23, {}};
    CHECK_FALSE(data.legal(b));
  }

  SUBCASE("Block touching chip x") {
    block b = {0, 3, 13, 17, 2, {}};
    CHECK_FALSE(data.legal(b));
  }

  SUBCASE("Block touching chip y") {
    block b = {0, 3, 13, 3, 7, {}};
    CHECK_FALSE(data.legal(b));
  }

  SUBCASE("Block overflowing chip x") {
    block b = {0, 3, 13, UINT32_MAX - 1, 7, {}};
    CHECK_FALSE(data.legal(b));
  }

  SUBCASE("Block overflowing chip y") {
    block b = {0, 3, 13, 3, UINT32_MAX - 1, {}};
    CHECK_FALSE(data.legal(b));
  }

  SUBCASE("Block larger than chip") {
    block b = {0, 0, 0, 40, 40, {}};
    CHECK_FALSE(data.legal(b));
    b = {1, 15, 15, 30, 30, {}};
    CHECK_FALSE(data.legal(b));
  }
}

// Test get_index_from_pos
TEST_CASE("Test get_index_from_pos") {
  Data data(20, 20);
  data.add_net({0, {}});
  data.add_net({2, {}});
  data.add_net({3, {}});
  REQUIRE_EQ(data.num_nets, 3);

  data.add_block({10, 0, 0, 2, 3, {0, 3}});
  data.add_block({11, 5, 0, 2, 2, {0}});
  data.add_block({12, 4, 2, 1, 1, {0, 2}});
  data.add_block({13, 9, 9, 2, 3, {2, 3}});
  REQUIRE_EQ(data.num_blocks, 4);

  CHECK_EQ(data.get_index_from_pos(0, 0), 0);
  CHECK_EQ(data.get_index_from_pos(5, 0), 1);
  CHECK_EQ(data.get_index_from_pos(4, 2), 2);
  CHECK_EQ(data.get_index_from_pos(9, 9), 3);
}

// Test try_move
TEST_CASE("Test try_move()") {
  Data data(20, 20);
  data.add_net({0, {}});
  data.add_net({2, {}});
  data.add_net({3, {}});
  REQUIRE_EQ(data.num_nets, 3);

  data.add_block({10, 0, 0, 2, 3, {0, 3}});
  data.add_block({11, 5, 0, 2, 2, {0}});
  data.add_block({12, 4, 2, 1, 1, {0, 2}});
  data.add_block({13, 9, 9, 2, 3, {2, 3}});
  REQUIRE_EQ(data.num_blocks, 4);

	SUBCASE("Legal move") {
	}

SUBCASE("Move outside chip x negative") {
	}

SUBCASE("Move outside chip x positive") {
	}

SUBCASE("Move outside chip y negative") {
	}

SUBCASE("Move outside chip y positive") {
	}


	SUBCASE("Cutting other block") {
	}

}

// Test try_swap

// Test try_rot_cw

// Test try_rot_cc

// Test try_flip_h

// Test try_flip_v
