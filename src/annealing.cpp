#include "../include/annealing.h"
#include <cstddef>
#include <cstdint>
#include <vector>

// Select a random move. If the move is a shift, it will be in the range
// [-window, window]
// NOTE: window_x and window_y should be smaller than INT32_MAX and larger than
// INT32_MIN, but that should not be an issue in real life use cases. Even if
// one unit represents one nanometer. INT32_MAX would be over 2 meters.
bool try_random_move(Data &data, uint32_t window_x, uint32_t window_y) {
  // Select move
  move move = static_cast<enum move>(xo_next() % LAST);
  uint64_t range;
  int32_t x_move;
  int32_t y_move;

  switch (move) {
  case SHIFT: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    range = 2 * window_x + 1;
    x_move = static_cast<int32_t>((xo_next() % range)) -
             static_cast<int32_t>(window_x);
    range = 2 * window_y + 1;
    y_move = static_cast<int32_t>((xo_next() % range)) -
             static_cast<int32_t>(window_y);
    return data.try_shift(b, x_move, y_move);
  }
  case SWAP: {
    block &b1 = data.get_block_by_index(xo_next() % data.num_blocks);
    block &b2 = data.get_block_by_index(xo_next() % data.num_blocks);
    return data.try_swap(b1, b2);
  }
  case FLIP_H: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    return data.try_flip_h(b);
  }
  case FLIP_V: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    return data.try_flip_v(b);
  }
  case ROT_CW: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    return data.try_rot_cw(b);
  }
  case ROT_CC: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    return data.try_rot_cc(b);
  }
  default:
    // How?
    panic("Somehow a non-existing type of move was selected");
    break;
  }
}

uint64_t hpwl_net(net &net) {

  if (net.pins.size() <= 1) {
    // Weird net, but ok
    return 0;
  }

  uint32_t min_x = UINT32_MAX;
  uint32_t max_x = 0;
  uint32_t min_y = UINT32_MAX;
  uint32_t max_y = 0;

  for (size_t i = 0; i < net.pins.size(); i++) {
    auto [id, n_x, n_y] = net.pins[i];
    min_x = (n_x < min_x) ? n_x : min_x;
    max_x = (n_x > max_x) ? n_x : max_x;
    min_y = (n_y < min_y) ? n_y : min_y;
    max_y = (n_y > max_y) ? n_y : max_y;
  }
  return (max_x - min_x) + (max_y - min_y);
}

uint64_t hpwl(Data &data) {
  uint64_t cost = 0;
  for (size_t i = 0; i < data.num_nets; i++) {
    cost += hpwl_net(data.get_net_by_index(i));
  }
  return cost;
}

uint64_t anneal(Data &data, uint64_t initial_temp, uint64_t final_temp,
                uint32_t initial_window_x, uint32_t final_window_x,
                uint32_t initial_window_y, uint32_t final_window_y,
                uint64_t steps, uint64_t warmup_steps, uint64_t tuning_steps,
                uint32_t moves_per_step, bool logging_enabled, log logger) {

  // Initialize random number gen
  // TODO: Do some real initialization
  xo_s[0] = 42;
  xo_s[1] = 42;
  xo_s[2] = 42;
  xo_s[3] = 42;

  // Best result of all time
  std::vector<block> best_result_blocks = {};
  std::vector<net> best_result_nets = {};
  best_result_blocks.reserve(data.num_blocks);
  best_result_nets.reserve(data.num_nets);
  uint64_t best_cost = UINT64_MAX;

  uint64_t cost;
  uint64_t current_cost = hpwl(data);
  uint64_t temp = initial_temp;
  uint32_t window_x = initial_window_x;
  uint32_t window_y = initial_window_y;

  uint64_t temp_reduction_interval = steps / (initial_temp - final_temp);
  uint64_t temp_reduction_counter = temp_reduction_interval;
  uint64_t temp_reduction_amount = ((initial_temp - final_temp) / steps) > 0
                                       ? ((initial_temp - final_temp) / steps)
                                       : 1;

  uint64_t window_x_reduction_interval =
      steps / (initial_window_x - final_window_x);
  uint64_t window_x_reduction_counter = window_x_reduction_interval;
  uint64_t window_x_reduction_amount =
      ((initial_window_x - final_window_x) / steps) > 0
          ? ((initial_window_x - final_window_x) / steps)
          : 1;

  uint64_t window_y_reduction_interval =
      steps / (initial_window_y - final_window_y);
  uint64_t window_y_reduction_counter = window_y_reduction_interval;
  uint64_t window_y_reduction_amount =
      ((initial_window_y - final_window_y) / steps) > 0
          ? ((initial_window_y - final_window_y) / steps)
          : 1;

  for (uint64_t i = 0; i < steps; i++) {

    // 1. Save state
    data.save_state();

    // 2. Perform moves
    uint64_t successful_moves = 0;
    while (successful_moves < moves_per_step) {
      successful_moves += try_random_move(data, window_x, window_y) ? 1 : 0;
    }

    // 3. Compute cost
    cost = hpwl(data);

    // 4. Decide if accept
    // If the cost is lower, we always accept
    if (cost > current_cost) {
      if (xo_next() % MAX_TEMP < temp) {
        // Moves are accepted
        current_cost = cost;
      } else {
        data.reset_state();
      }
    }

    // 5. Update best configuration if warm-up is done
    else if (cost < best_cost && i >= warmup_steps) {
      data.save_best();
      best_cost = cost;
      current_cost = cost;
      // Accept moves if cost is lower
    } else {
      current_cost = cost;
    }

    // 6. Update temperature and windows
    if (temp_reduction_counter == 0) {
      temp -= temp_reduction_amount;
      temp_reduction_counter = temp_reduction_interval;
    }
    if (window_x_reduction_counter == 0) {
      window_x -= window_x_reduction_amount;
      window_x_reduction_counter = window_x_reduction_interval;
    }
    if (window_y_reduction_counter == 0) {
      window_y -= window_y_reduction_amount;
      window_y_reduction_counter = window_y_reduction_interval;
    }
  }

  bool tuning_found_improvement = false;
  // Tuning steps
  for (uint64_t i = 0; i < tuning_steps; i++) {

    // 1. Save state
    data.save_state();

    // 2. Perform moves
    uint64_t successful_moves = 0;
    while (successful_moves < moves_per_step) {
      successful_moves += try_random_move(data, window_x, window_y) ? 1 : 0;
    }

    // 3. Compute cost
    cost = hpwl(data);

    // 4. Decide if accept
    // If the cost is lower, we always accept
    if (cost < best_cost) {
      // Moves are accepted
      best_cost = cost;
      tuning_found_improvement = true;
    } else {
      data.reset_state();
    }
  }

  // If tuning was performed the current cost can be lower than the best_cost
  // found earlier, but the state wasn't saved to best state
  if (tuning_found_improvement) {
    data.save_best();
  }
  return best_cost;
}
