#include "../include/annealing.h"
#include <cstddef>
#include <cstdint>
#include <random>
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
    std::cout << "Try shift of block " << b.id << " with x " << x_move << " y "
              << y_move << std::endl;
    return data.try_shift(b, x_move, y_move);
  }
  case SWAP: {
    block &b1 = data.get_block_by_index(xo_next() % data.num_blocks);
    block &b2 = data.get_block_by_index(xo_next() % data.num_blocks);
    std::cout << "Try swapping block " << b1.id << " and " << b1.id
              << std::endl;
    return data.try_swap(b1, b2);
  }
  case FLIP_H: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    std::cout << "Try flip_h block " << b.id << std::endl;
    return data.try_flip_h(b);
  }
  case FLIP_V: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    std::cout << "Try flip_v block " << b.id << std::endl;
    return data.try_flip_v(b);
  }
  case ROT_CW: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    std::cout << "Try rot_cw block " << b.id << std::endl;
    return data.try_rot_cw(b);
  }
  case ROT_CC: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    std::cout << "Try rot_cc block " << b.id << std::endl;
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
                uint32_t moves_per_step, bool logging_enabled,
                struct log logger) {

  // Initialize random number gen
  // TODO: Do some real initialization
  std::random_device rd;
  std::mt19937_64 init(rd());

  xo_init_state(init(), init(), init(), init());

  // Best result of all time
  std::vector<block> best_result_blocks = {};
  std::vector<net> best_result_nets = {};
  best_result_blocks.reserve(data.num_blocks);
  best_result_nets.reserve(data.num_nets);
  uint64_t best_cost = UINT64_MAX;

  // for (int i = 0; i < 20; i++) {
  //   std::cout << init() << std::endl;
  //   std::cout << "-----" << std::endl;
  //   std::cout << xo_next() << std::endl;
  // }
  // return 0;

  uint64_t cost;
  uint64_t current_cost = hpwl(data);
  uint64_t temp = initial_temp;
  uint32_t window_x = initial_window_x;
  uint32_t window_y = initial_window_y;

  uint64_t temp_reduction_interval = (steps / (initial_temp - final_temp)) > 0
                                         ? steps / (initial_temp - final_temp)
                                         : 1;
  uint64_t temp_reduction_counter = temp_reduction_interval;
  uint64_t temp_reduction_amount = ((initial_temp - final_temp) / steps) > 0
                                       ? ((initial_temp - final_temp) / steps)
                                       : 1;

  uint64_t window_x_reduction_interval =
      (steps / (initial_window_x - final_window_x)) > 0
          ? (steps / (initial_window_x - final_window_x))
          : 1;
  uint64_t window_x_reduction_counter = window_x_reduction_interval;
  uint64_t window_x_reduction_amount =
      ((initial_window_x - final_window_x) / steps) > 0
          ? ((initial_window_x - final_window_x) / steps)
          : 1;

  uint64_t window_y_reduction_interval =
      (steps / (initial_window_y - final_window_y)) > 0
          ? (steps / (initial_window_y - final_window_y))
          : 1;
  uint64_t window_y_reduction_counter = window_y_reduction_interval;
  uint64_t window_y_reduction_amount =
      ((initial_window_y - final_window_y) / steps) > 0
          ? ((initial_window_y - final_window_y) / steps)
          : 1;

  uint64_t logging_counter = logger.interval > 0 ? logger.interval : 1;

  std::cout << "Finished initialization. Starting main loop" << std::endl;
  std::cout << "Initial temperature: " << temp << std::endl;
  std::cout << "Final temperature: " << final_temp << std::endl;
  std::cout << "Temperature reduction interval: " << temp_reduction_interval << std::endl;
  std::cout << "Temperature reduction amount: " << temp_reduction_amount << std::endl;

  std::cout << "Initial window x: " << window_x << std::endl;
  std::cout << "Final window x: " << final_window_x << std::endl;
  std::cout << "window x reduction interval: " << window_x_reduction_interval << std::endl;
  std::cout << "window x reduction amount: " << window_x_reduction_amount << std::endl;

  std::cout << "Initial window y: " << window_y << std::endl;
  std::cout << "Final window y: " << final_window_y << std::endl;
  std::cout << "window y reduction interval: " << window_y_reduction_interval << std::endl;
  std::cout << "window y reduction amount: " << window_y_reduction_amount << std::endl;


  for (uint64_t i = 0; i < steps; i++) {

    std::cout << "Iteration " << i << std::endl;

    // 1. Save state
    data.save_state();
    std::cout << "Saved state " << std::endl;

    // 2. Perform moves
    uint64_t successful_moves = 0;
    while (successful_moves < moves_per_step) {
      successful_moves += try_random_move(data, window_x, window_y) ? 1 : 0;
      std::cout << successful_moves << " Successful moves" << std::endl;
    }

    // 3. Compute cost

    std::cout << "Computing cost" << std::endl;
    cost = hpwl(data);
    std::cout << "New cost: " << cost << std::endl;

    // 4. Decide if accept
    // If the cost is lower, we always accept
    if (cost > current_cost) {
      std::cout << "Larger than current cost" << std::endl;
      if (xo_next() % MAX_TEMP < temp) {
        std::cout << "Accept anyways" << std::endl;
        // Moves are accepted
        current_cost = cost;
      } else {
        std::cout << "Didn't accept. Resetting..." << std::endl;
        data.reset_state();
      }
    }

    // 5. Update best configuration if warm-up is done
    else if (cost < best_cost && i >= warmup_steps) {
      std::cout << "Updating best configuration" << std::endl;
      data.save_best();
      best_cost = cost;
      current_cost = cost;
      // Accept moves if cost is lower
    } else {
      current_cost = cost;
    }

    // 6. Update temperature and windows
    temp_reduction_counter--;
    window_x_reduction_counter--;
    window_y_reduction_counter--;

    if (temp_reduction_counter == 0) {
      temp -= temp_reduction_amount;
      temp_reduction_counter = temp_reduction_interval;
      std::cout << "Reducing temperature to " << temp << std::endl;
    }

    if (window_x_reduction_counter == 0) {
      window_x -= window_x_reduction_amount;
      window_x_reduction_counter = window_x_reduction_interval;
      std::cout << "Reducing window x to " << window_x << std::endl;
    }
    if (window_y_reduction_counter == 0) {
      window_y -= window_y_reduction_amount;
      window_y_reduction_counter = window_y_reduction_interval;
      std::cout << "Reducing window y to " << window_y << std::endl;
    }

    // 7. Log
    logging_counter--;
    if (logging_counter == 0) {
      std::cout << "Logging" << std::endl;
      logger.step = i;
      save_pgm(data, logger);
      logging_counter = logger.interval;
    }
  }

  bool tuning_found_improvement = false;
  // Tuning steps
  for (uint64_t i = steps; i < steps + tuning_steps; i++) {
    std::cout << "Tuning iteration " << i << std::endl;

    // 1. Save state
    std::cout << "Saved state " << std::endl;
    data.save_state();

    // 2. Perform moves
    uint64_t successful_moves = 0;
    while (successful_moves < moves_per_step) {
      successful_moves += try_random_move(data, window_x, window_y) ? 1 : 0;
      std::cout << successful_moves << " Successful moves" << std::endl;
    }

    // 3. Compute cost
    std::cout << "Computing cost" << std::endl;
    cost = hpwl(data);
    std::cout << "New cost: " << cost << std::endl;

    // 4. Decide if accept
    // If the cost is lower, we always accept
    if (cost < best_cost) {
      // Moves are accepted
      std::cout << "Found improvement" << std::endl;
      best_cost = cost;
      tuning_found_improvement = true;
    } else {
      std::cout << "Larger than current cost. Resetting..." << std::endl;
      data.reset_state();
    }

    // 5. Log
    logging_counter--;
    if (logging_counter == 0) {
      std::cout << "Logging" << std::endl;
      logger.step = i;
      save_pgm(data, logger);
      logging_counter = logger.interval;
    }
  }

  // If tuning was performed the current cost can be lower than the best_cost
  // found earlier, but the state wasn't saved to best state
  if (tuning_found_improvement) {
    std::cout << "Tuning found an improvement" << std::endl;
    data.save_best();
  }
  return best_cost;
}
