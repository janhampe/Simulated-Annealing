#include "../include/data.h"
#include "doctest.h"
#include <cstdint>

// Data Tests
// NOTE: Because the 1 unit distance to the edge requirement was added later,
// the block with id is technically in an illegal position. This is not checked,
// because if find_initial_placement isn't called by the user, the block
// placement is their responsibility. To cut it short, as long as the block
// isn't checked for legality in its current position, this goes unnoticed.

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

// Test try_shift
TEST_CASE("Test try_shift()") {
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

  SUBCASE("Legal move positive") {
    block &b = data.get_block_by_id(10);
    CHECK(data.try_shift(b, 15, 16));
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 15);
        CHECK_EQ(y, 16);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 15);
        CHECK_EQ(y, 16);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  SUBCASE("Legal move negative") {
    block &b = data.get_block_by_id(13);
    CHECK(data.try_shift(b, -7, -2));
    SUBCASE("Check pins on net 2") {
      net &n = data.get_net_by_id(2);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 2);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 7);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 0);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 7);
      }
    }
  }

  SUBCASE("Move outside chip x negative") {
    block &b = data.get_block_by_id(10);
    CHECK_FALSE(data.try_shift(b, -5, 16));
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 0);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 0);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  SUBCASE("Move outside chip x positive") {
    block &b = data.get_block_by_id(10);
    CHECK_FALSE(data.try_shift(b, 35, 10));
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 0);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 0);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  SUBCASE("Move outside chip y negative") {
    block &b = data.get_block_by_id(10);
    CHECK_FALSE(data.try_shift(b, 5, -1));
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 0);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 0);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  SUBCASE("Move outside chip y positive") {
    block &b = data.get_block_by_id(10);
    CHECK_FALSE(data.try_shift(b, 5, 23));
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 0);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 0);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  SUBCASE("Cutting other block") {
    block &b = data.get_block_by_id(10);
    CHECK_FALSE(data.try_shift(b, 3, 0));
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 0);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 0);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }
}

// Test try_swap
TEST_CASE("Test try_swap") {
  Data data(20, 20);
  data.add_net({0, {}});
  data.add_net({2, {}});
  data.add_net({3, {}});
  REQUIRE_EQ(data.num_nets, 3);

  data.add_block({10, 1, 1, 2, 3, {0, 3}});
  data.add_block({11, 5, 0, 2, 2, {0}});
  data.add_block({12, 4, 2, 1, 1, {0, 2}});
  data.add_block({13, 9, 9, 2, 3, {2, 3}});
  REQUIRE_EQ(data.num_blocks, 4);

  SUBCASE("Legal swap") {
    block &a = data.get_block_by_id(10);
    block &b = data.get_block_by_id(13);
    CHECK(data.try_swap(a, b));
    CHECK_EQ(a.x, 9);
    CHECK_EQ(a.y, 9);
    CHECK_EQ(b.x, 1);
    CHECK_EQ(b.y, 1);

    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 0);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 2") {
      net &n = data.get_net_by_id(2);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 2);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 1);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 1);
      }
    }
  }
}

// Test try_rot_cw
TEST_CASE("Test try_rot_cw()") {
  Data data(20, 20);
  data.add_net({0, {}});
  data.add_net({2, {}});
  data.add_net({3, {}});
  REQUIRE_EQ(data.num_nets, 3);

  data.add_block({10, 1, 1, 2, 3, {0, 3}});
  data.add_block({11, 2, 6, 2, 2, {0}});
  data.add_block({12, 5, 2, 1, 1, {0, 2}});
  data.add_block({13, 9, 9, 2, 3, {2, 3}});
  REQUIRE_EQ(data.num_blocks, 4);

  block &b = data.get_block_by_id(10);
  CHECK(data.try_rot_cw(b));

  SUBCASE("Check pins at 90°") {
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 6);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  CHECK(data.try_rot_cw(b));

  SUBCASE("Check pins at 180°") {
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 3);
        CHECK_EQ(y, 4);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 6);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 3);
        CHECK_EQ(y, 4);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  CHECK(data.try_rot_cw(b));

  SUBCASE("Check pins at 270°") {
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 3);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 6);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 3);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  CHECK(data.try_rot_cw(b));

  SUBCASE("Check pins at 360°") {
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 6);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }
}

// Test try_rot_cc
TEST_CASE("Test try_rot_cc()") {
  Data data(20, 20);
  data.add_net({0, {}});
  data.add_net({2, {}});
  data.add_net({3, {}});
  REQUIRE_EQ(data.num_nets, 3);

  data.add_block({10, 1, 1, 2, 3, {0, 3}});
  data.add_block({11, 2, 6, 2, 2, {0}});
  data.add_block({12, 5, 2, 1, 1, {0, 2}});
  data.add_block({13, 9, 9, 2, 3, {2, 3}});
  REQUIRE_EQ(data.num_blocks, 4);

  block &b = data.get_block_by_id(10);
  CHECK(data.try_rot_cc(b));

  SUBCASE("Check pins at -90°") {
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 3);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 6);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 3);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  CHECK(data.try_rot_cc(b));

  SUBCASE("Check pins at -180°") {
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 3);
        CHECK_EQ(y, 4);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 6);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 3);
        CHECK_EQ(y, 4);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  CHECK(data.try_rot_cc(b));

  SUBCASE("Check pins at -270°") {
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 6);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 4);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  CHECK(data.try_rot_cc(b));

  SUBCASE("Check pins at -360°") {
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 6);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }
}

// Test try_flip_h
TEST_CASE("Test try_flip_h()") {
  Data data(20, 20);
  data.add_net({0, {}});
  data.add_net({2, {}});
  data.add_net({3, {}});
  REQUIRE_EQ(data.num_nets, 3);

  data.add_block({10, 1, 1, 2, 3, {0, 3}});
  data.add_block({11, 2, 6, 2, 2, {0}});
  data.add_block({12, 5, 2, 1, 1, {0, 2}});
  data.add_block({13, 9, 9, 2, 3, {2, 3}});
  REQUIRE_EQ(data.num_blocks, 4);

  block &b = data.get_block_by_id(10);
  CHECK(data.try_flip_h(b));

  SUBCASE("Test flip") {
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 3);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 6);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 3);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  CHECK(data.try_flip_h(b));

  SUBCASE("Test flip back") {
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 6);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }
}

// Test try_flip_v
TEST_CASE("Test try_flip_v()") {
  Data data(20, 20);
  data.add_net({0, {}});
  data.add_net({2, {}});
  data.add_net({3, {}});
  REQUIRE_EQ(data.num_nets, 3);

  data.add_block({10, 1, 1, 2, 3, {0, 3}});
  data.add_block({11, 2, 6, 2, 2, {0}});
  data.add_block({12, 5, 2, 1, 1, {0, 2}});
  data.add_block({13, 9, 9, 2, 3, {2, 3}});
  REQUIRE_EQ(data.num_blocks, 4);

  block &b = data.get_block_by_id(10);
  CHECK(data.try_flip_v(b));

  SUBCASE("Test flip") {
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 4);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 6);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 4);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }

  CHECK(data.try_flip_v(b));

  SUBCASE("Test flip back") {
    SUBCASE("Check pins on net 0") {
      net &n = data.get_net_by_id(0);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 11);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, 6);
      }

      {
        auto [id, x, y] = n.pins[2];
        CHECK_EQ(id, 12);
        CHECK_EQ(x, 5);
        CHECK_EQ(y, 2);
      }
    }

    SUBCASE("Check pins on net 3") {
      net &n = data.get_net_by_id(3);
      {
        auto [id, x, y] = n.pins[0];
        CHECK_EQ(id, 10);
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 1);
      }

      {
        auto [id, x, y] = n.pins[1];
        CHECK_EQ(id, 13);
        CHECK_EQ(x, 9);
        CHECK_EQ(y, 9);
      }
    }
  }
}
