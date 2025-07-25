#include "data.h"
#include "ui.h"
#include "xoshiro256pp.h"
#include <cstdint>
#include <functional>

#define MAX_TEMP 1'000'000'000'000

enum move { SHIFT, SWAP, FLIP_H, FLIP_V, ROT_CW, ROT_CC, LAST };

bool try_random_move(Data &blocks, uint32_t window_x, uint32_t window_y);

uint64_t hpwl_net(net &net);

uint64_t hpwl(Data &data);

uint64_t mcl_net(net &net);

uint64_t mcl(Data &data);

uint64_t star_net(net &net);

uint64_t star(Data &data);

// NOTE: Maximum temperature is 1'000'000'000'000. This means that 100% of
// changes are accpeted, even if they have a worse cost. Consequently a
// temperature of 0 results in only improving moves being accepted. Tuning steps
// are additional steps performed with the final window sizes, but only
// improvements are accepted
// Warm-up steps are the number of steps performed, before checking and saving
// the best current result. This can be used to increase performance when the
// very bad initial placement is massively improved.
uint64_t anneal(Data &data, std::function<uint64_t(Data&)> cost_fn, uint64_t initial_temp, uint64_t final_temp,
                uint32_t initial_window_x, uint32_t final_window_x,
                uint32_t initial_window_y, uint32_t final_window_y,
                uint64_t steps, uint64_t warmup_steps, uint64_t tuning_steps,
                uint32_t initial_moves_per_step, uint32_t final_moves_per_step,
                bool logging_enabled, log logger);
