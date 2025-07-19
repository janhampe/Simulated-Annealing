#include "../include/data.h"
#include "../include/ui.h"
#include "doctest.h"

// Placer Tests
TEST_CASE("Simple placement") {
  Data data(2000, 2000);

  data.add_net({0, {}});
  data.add_net({2, {}});
  data.add_net({3, {}});
  REQUIRE_EQ(data.num_nets, 3);

  data.add_block({10, 0, 0, 200, 300, {0, 3}});
  data.add_block({11, 500, 0, 200, 200, {0}});
  data.add_block({12, 400, 200, 100, 100, {0, 2}});
  data.add_block({13, 900, 900, 200, 300, {2, 3}});
  REQUIRE_EQ(data.num_blocks, 4);

  CHECK(data.find_initial_placement());
  data.get_block_by_id(13);
  save_pgm(data, "", "test", 0, 1);
}
