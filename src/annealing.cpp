#include "../include/annealing.h"
#include "../include/debug.h"
#include "../include/panic.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <random>
#include <tuple>
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
    DEBUG("Try shift of block ", b.id, " with x ", x_move, " y ", y_move)
    return data.try_shift(b, x_move, y_move);
  }
  case SWAP: {
    block &b1 = data.get_block_by_index(xo_next() % data.num_blocks);
    block &b2 = data.get_block_by_index(xo_next() % data.num_blocks);
    DEBUG("Try swapping block ", b1.id, " and ", b2.id)
    return data.try_swap(b1, b2);
  }
  case FLIP_H: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    DEBUG("Try flip_h block ", b.id)
    return data.try_flip_h(b);
  }
  case FLIP_V: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    DEBUG("Try flip_v block ", b.id)
    return data.try_flip_v(b);
  }
  case ROT_CW: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    DEBUG("Try rot_cw block ", b.id)
    return data.try_rot_cw(b);
  }
  case ROT_CC: {
    block &b = data.get_block_by_index(xo_next() % data.num_blocks);
    DEBUG("Try rot_cc block ", b.id)
    return data.try_rot_cc(b);
  }
  default:
    // How?
    panic("Somehow a non-existing type of move was selected");
    break;
  }
}

uint64_t mcl_net(net &net) {
  if (net.pins.size() <= 1) {
    return 0;
  }
  std::sort(net.pins.begin(), net.pins.end(),
            [](std::tuple<uint64_t, uint32_t, uint32_t> &a,
               std::tuple<uint64_t, uint32_t, uint32_t> &b) {
              return std::get<1>(a) < std::get<1>(b);
            });
  uint64_t cost = 0;
  uint32_t current_x = get<1>(net.pins[0]);
  uint32_t current_y = get<2>(net.pins[0]);
  uint32_t x;
  uint32_t y;

  for (size_t i = 1; i < net.pins.size(); i++) {
    x = std::get<1>(net.pins[i]);
    y = std::get<2>(net.pins[i]);
    cost += (x > current_x) ? (x - current_x) : (current_x - x);
    cost += (y > current_y) ? (y - current_y) : (current_y - y);
  }
  return cost;
}

uint64_t mcl(Data &data) {
  uint64_t cost = 0;
  for (size_t i = 0; i < data.num_nets; i++) {
    cost += mcl_net(data.get_net_by_index(i));
  }
  return cost;
}

uint64_t star_net(net &net) {
  if (net.pins.size() <= 1) {
    return 0;
  }
  uint64_t cost = 0;
  uint32_t root_x = std::get<1>(net.pins[0]);
  uint32_t root_y = std::get<2>(net.pins[0]);
  uint32_t x;
  uint32_t y;

  for (size_t i = 1; i < net.pins.size(); i++) {
    x = std::get<1>(net.pins[i]);
    y = std::get<2>(net.pins[i]);
    cost += (x > root_x) ? (x - root_x) : (root_x - x);
    cost += (y > root_y) ? (y - root_y) : (root_y - y);
  }
  return cost;
}

uint64_t star(Data &data) {
  uint64_t cost = 0;
  for (size_t i = 0; i < data.num_nets; i++) {
    cost += star_net(data.get_net_by_index(i));
  }
  return cost;
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

uint64_t anneal(Data &data, std::function<uint64_t(Data &)> cost_fn,
                uint64_t initial_temp, uint64_t final_temp,
                uint32_t initial_window_x, uint32_t final_window_x,
                uint32_t initial_window_y, uint32_t final_window_y,
                uint64_t steps, uint64_t warmup_steps, uint64_t tuning_steps,
                uint32_t initial_moves_per_step, uint32_t final_moves_per_step,
                bool logging_enabled, struct log logger) {

  // Initialize random number gen
  std::random_device rd;
  std::mt19937_64 init(rd());

  xo_init_state(init(), init(), init(), init());

  uint64_t cost;
  uint64_t current_cost = cost_fn(data);
  uint64_t best_cost = current_cost;
  data.save_best();
  uint64_t temp = initial_temp;
  uint32_t window_x = initial_window_x;
  uint32_t window_y = initial_window_y;
  uint32_t moves_per_step = initial_moves_per_step;

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

  uint64_t move_reduction_interval =
      (steps / (initial_moves_per_step - final_moves_per_step)) > 0
          ? (steps / (initial_moves_per_step - final_moves_per_step))
          : 1;
  uint64_t move_reduction_counter = move_reduction_interval;
  uint64_t move_reduction_amount =
      ((initial_moves_per_step - final_moves_per_step) / steps) > 0
          ? ((initial_moves_per_step - final_moves_per_step) / steps)
          : 1;

  uint64_t logging_counter = logger.interval > 0 ? logger.interval : 1;

  DEBUG("Finished initialization. Starting main loop")
  DEBUG("Initial temperature: ", temp)
  DEBUG("Final temperature: ", final_temp)
  DEBUG("Temperature reduction interval: ", temp_reduction_interval)
  DEBUG("Temperature reduction amount: ", temp_reduction_amount)

  DEBUG("Initial window x: ", window_x)
  DEBUG("Final window x: ", final_window_x)
  DEBUG("window x reduction interval: ", window_x_reduction_interval)
  DEBUG("window x reduction amount: ", window_x_reduction_amount)

  DEBUG("Initial window y: ", window_y)
  DEBUG("Final window y: ", final_window_y)
  DEBUG("window y reduction interval: ", window_y_reduction_interval)
  DEBUG("window y reduction amount: ", window_y_reduction_amount)

  DEBUG("Initial moves per step: ", initial_moves_per_step)
  DEBUG("Final moves per step: ", final_moves_per_step)
  DEBUG("Move reduction interval: ", move_reduction_interval)
  DEBUG("Move reduction amount: ", move_reduction_amount)

  for (uint64_t i = 0; i < steps; i++) {

    // 1. Save state
    data.save_state();
    DEBUG("Saved state ")

    // 2. Perform moves
    uint64_t successful_moves = 0;
    [[maybe_unused]]
    uint64_t move_failures = 0;
    while (successful_moves < moves_per_step) {
      successful_moves += try_random_move(data, window_x, window_y) ? 1 : 0;
      if (try_random_move(data, window_x, window_y)) {
        successful_moves++;
      } else {
        move_failures++;
      }
      DEBUG(successful_moves, " Successful moves")
    }
    DEBUG(move_failures, " move failures for ", successful_moves,
          " successful moves")

    // 3. Compute cost

    DEBUG("Computing cost")
    cost = cost_fn(data);
    DEBUG("New cost: ", cost)

    // 4. Decide if accept
    // If the cost is lower, we always accept
    if (cost > current_cost) {
      DEBUG("Larger than current cost")
      if (xo_next() % MAX_TEMP < temp) {
        DEBUG("Accept anyways")
        // Moves are accepted
        current_cost = cost;
      } else {
        DEBUG("Didn't accept. Resetting...")
        data.reset_state();
      }
    }

    // 5. Update best configuration if warm-up is done
    else if (cost < best_cost && i >= warmup_steps) {
      DEBUG("Updating best configuration")
      data.save_best();
      best_cost = cost;
      current_cost = cost;
      // Accept moves if cost is lower
    } else {
      current_cost = cost;
    }

    // 6. Update temperature, windows and moves
    temp_reduction_counter--;
    window_x_reduction_counter--;
    window_y_reduction_counter--;
    move_reduction_counter--;

    if (temp_reduction_counter == 0) {
      temp -= temp_reduction_amount;
      temp_reduction_counter = temp_reduction_interval;
      DEBUG("Reducing temperature to ", temp)
    }

    if (window_x_reduction_counter == 0) {
      window_x -= window_x_reduction_amount;
      window_x_reduction_counter = window_x_reduction_interval;
      DEBUG("Reducing window x to ", window_x)
    }
    if (window_y_reduction_counter == 0) {
      window_y -= window_y_reduction_amount;
      window_y_reduction_counter = window_y_reduction_interval;
      DEBUG("Reducing window y to ", window_y)
    }

    if (move_reduction_counter == 0) {
      moves_per_step -= move_reduction_amount;
      move_reduction_counter = move_reduction_interval;
      DEBUG("Reducing moves per step to ", moves_per_step)
    }

    // 7. Log
    logging_counter--;
    if (logging_counter == 0) {
      DEBUG("Logging")
      LOG_INFO("Iteration ", i)
      LOG_INFO("Current cost ", current_cost)
      LOG_INFO("Best ever cost ", best_cost)
      logger.step = i;
      save_pgm(data, logger);
      logging_counter = logger.interval;
    }
  }

  bool tuning_found_improvement = false;
  // Tuning steps
  for (uint64_t i = steps; i < steps + tuning_steps; i++) {

    // 1. Save state
    DEBUG("Saved state ")
    data.save_state();

    // 2. Perform moves
    uint64_t successful_moves = 0;
    while (successful_moves < moves_per_step) {
      successful_moves += try_random_move(data, window_x, window_y) ? 1 : 0;
      DEBUG(successful_moves, " Successful moves")
    }

    // 3. Compute cost
    DEBUG("Computing cost")
    cost = cost_fn(data);
    DEBUG("New cost: ", cost)

    // 4. Decide if accept
    // If the cost is lower, we always accept
    if (cost < best_cost) {
      // Moves are accepted
      DEBUG("Found improvement")
      best_cost = cost;
      tuning_found_improvement = true;
    } else {
      DEBUG("Larger than current cost. Resetting...")
      data.reset_state();
    }

    // 5. Log
    logging_counter--;
    if (logging_counter == 0) {
      DEBUG("Logging")
      LOG_INFO("Tuing Iteration ", i)
      LOG_INFO("Current cost ", current_cost)
      LOG_INFO("Best ever cost ", best_cost)
      logger.step = i;
      save_pgm(data, logger);
      logging_counter = logger.interval;
    }
  }

  // If tuning was performed the current cost can be lower than the best_cost
  // found earlier, but the state wasn't saved to best state
  if (tuning_found_improvement) {
    LOG_INFO("Tuning found an improvement")
    data.save_best();
  }
  return best_cost;
}
