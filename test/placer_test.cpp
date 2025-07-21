#include "../include/data.h"
#include "../include/ui.h"
#include "doctest.h"

// Placer Tests
TEST_CASE("Simple placement") {
  Data data(500, 505);

  data.add_net({0, {}});
  data.add_net({2, {}});
  data.add_net({3, {}});
  REQUIRE_EQ(data.num_nets, 3);

  data.add_block({10, 0, 0, 200, 300, {0, 3}});
  data.add_block({11, 0, 0, 200, 200, {0}});
  data.add_block({12, 0, 200, 100, 100, {0, 2}});
  data.add_block({13, 900, 900, 200, 300, {2, 3}});
  data.add_block({14, 0, 0, 100, 100, {0, 3}});
  data.add_block({15, 0, 0, 60, 75, {0, 3}});
  // data.add_block({16, 0, 0, 20, 20, {0, 3}});
  // data.add_block({17, 0, 0, 32, 20, {0, 3}});
  // data.add_block({18, 0, 0, 25, 15, {0, 3}});
  REQUIRE_EQ(data.num_blocks, 6);

  CHECK(data.find_initial_placement());
  log logger = {"", "test", 0, 0, 1};
  save_pgm(data, logger);
}
