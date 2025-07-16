#include "data.h"
#include <cstdint>
#include <string>

enum move { SHIFT, SWAP, FLIP_H, FLIP_V, ROT_CW, ROT_CC };

bool try_random_move(Data &blocks, uint32_t window_x, uint32_t window_y);

uint64_t hpwl_net(Data& data, uint32_t net);

uint64_t hpwl(Data & data);

// TODO: Add logging to logfile
uint64_t anneal(Data &data, double initial_temp, double final_temp,
                uint32_t initial_window_x, uint32_t final_window_x,
                uint32_t initial_window_y, uint32_t final_window_y,
                uint64_t steps, uint32_t moves_per_step);
