#include "../include/annealing.h"
#include <cmath>
#include <cstdint>

int main() {

  // 1. Set everything up
  uint32_t chip_x = 500;
  uint32_t chip_y = 505;
  Data data = Data(chip_x, chip_y);
  struct log logger = {.dir_path = "log",
                       .file_prefix = "test",
                       .step = 0,
                       .interval = 1,
                       .units_per_pixel = 1};
  // 2. Read input

  data.add_net({0, {}});
  data.add_net({2, {}});
  data.add_net({3, {}});

  data.add_block({10, 0, 0, 200, 300, {0, 3}});
  data.add_block({11, 0, 0, 200, 200, {0}});
  data.add_block({12, 0, 200, 100, 100, {0, 2}});
  data.add_block({13, 900, 900, 200, 300, {2, 3}});
  data.add_block({14, 0, 0, 100, 100, {0, 3}});
  data.add_block({15, 0, 0, 60, 75, {0, 3}});

  data.find_initial_placement();
  // 3. annealing
  uint64_t initial_cost = hpwl(data);
  uint64_t final_cost;
  uint64_t initial_temp = MAX_TEMP / 2; // 50%
  uint64_t final_temp = MAX_TEMP / 20;  // 5%
  uint32_t initial_window_x = chip_x;
  uint32_t final_window_x = 5;
  uint32_t initial_window_y = chip_y;
  uint32_t final_window_y = 5;
  uint64_t steps = 10;
  uint64_t warmup_steps = 0;
  uint64_t tuning_steps = 5;
  uint32_t moves_per_step = 1;
  bool logging_enabled = true;

  std::cerr << "Initial cost: " << initial_cost << std::endl;

  final_cost =
      anneal(data, initial_temp, final_temp, initial_window_x, final_window_x,
             initial_window_y, final_window_y, steps, warmup_steps,
             tuning_steps, moves_per_step, logging_enabled, logger);
  // 4. present results
  save_pgm(data, logger);
  std::cerr << "Final cost: " << final_cost << std::endl;
  return 0;
}
