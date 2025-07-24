#include "../include/annealing.h"
#include "../include/cxxopts.hpp"
#include "../include/debug.h"
#include "../include/input.h"
#include "../include/panic.h"
#include <cmath>
#include <cstdint>
#include <fstream>
#include <string>

int main(int argc, char **argv) {

  // 1. Set everything up

  cxxopts::Options options("neal",
                           "Simulated annealing for genlib and verilog files");

  options.add_options()("g,genlib", "Genlib File",
                        cxxopts::value<std::string>())(
      "v,verilog", "Verilog File", cxxopts::value<std::string>())(
      "cx,chip_x", "Chip size in x-dimension",
      cxxopts::value<uint32_t>()->default_value("200"))(
      "cy,chip_y", "Chip size in y-dimension",
      cxxopts::value<uint32_t>()->default_value("200"))(
      "it,initial_temp", "Initial temperature",
      cxxopts::value<uint64_t>()->default_value("500000000000"))(
      "ft,final_temp", "Final temperature",
      cxxopts::value<uint64_t>()->default_value("500"))(
      "iwx, initial_window_x",
      "Initial maximum shift move distance in x-dimension",
      cxxopts::value<uint32_t>()->default_value("100"))(
      "fwx, final_window_x", "Final maximum shift move distance in x-dimension",
      cxxopts::value<uint32_t>()->default_value("1"))(
      "iwy, initial_window_y",
      "Initial maximum shift move distance in y-dimension",
      cxxopts::value<uint32_t>()->default_value("100"))(
      "fwy, final_window_y", "Final maximum shift move distance in y-dimension",
      cxxopts::value<uint32_t>()->default_value("1"))(
      "s,steps", "Number of annealing steps",
      cxxopts::value<uint64_t>()->default_value("0"))(
      "ws, warmup_steps", "Number of iterations before best result is saved",
      cxxopts::value<uint64_t>()->default_value("0"))(
      "ts, tuning_steps",
      "Number of iterations after annealing where "
      "only the a better result is accepted",
      cxxopts::value<uint64_t>()->default_value("0"))(
      "imps, initial_moves_per_step",
      "Initial number of individual moves executed per annealing step",
      cxxopts::value<uint32_t>()->default_value("50"))(
      "fmps, final_moves_per_step",
      "Final number of individual moves executed per annealing step",
      cxxopts::value<uint32_t>()->default_value("1"))(
      "ld,log_dir", "Path to log directory",
      cxxopts::value<std::string>()->default_value("log/"))(
      "lf,log_file", "File prefix for logs",
      cxxopts::value<std::string>()->default_value("log"))(
      "li,log_interval", "Number of iterations between logs",
      cxxopts::value<uint64_t>()->default_value("5"))("h,help", "Print usage");

  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    std::cout
        << "Maximum temperature, meaning a 100% chance of accepting a new "
           "configuration even if it is worse, is 1'000'000'000'000."
        << std::endl;
    exit(0);
  }

  uint32_t chip_x = result["chip_x"].as<uint32_t>();
  uint32_t chip_y = result["chip_y"].as<uint32_t>();
  uint64_t initial_temp = result["initial_temp"].as<uint64_t>(); // 50%
  uint64_t final_temp = result["final_temp"].as<uint64_t>();     // 0.5%
  uint32_t initial_window_x = result["initial_window_x"].as<uint32_t>();
  uint32_t final_window_x = result["final_window_x"].as<uint32_t>();
  uint32_t initial_window_y = result["initial_window_y"].as<uint32_t>();
  uint32_t final_window_y = result["final_window_y"].as<uint32_t>();
  uint64_t steps = result["steps"].as<uint64_t>();
  uint64_t warmup_steps = result["warmup_steps"].as<uint64_t>();
  uint64_t tuning_steps = result["tuning_steps"].as<uint64_t>();
  uint32_t initial_moves_per_step =
      result["initial_moves_per_step"].as<uint32_t>();
  uint32_t final_moves_per_step = result["final_moves_per_step"].as<uint32_t>();
  bool logging_enabled = true;
  Data data = Data(chip_x, chip_y);
  struct log logger = {.dir_path = result["log_dir"].as<std::string>(),
                       .file_prefix = result["log_file"].as<std::string>(),
                       .step = 0,
                       .interval = result["log_interval"].as<uint64_t>(),
                       .units_per_pixel = 1};
  // 2. Read input

  IO io(data);
  lorina::text_diagnostics text_dia;
  lorina::diagnostic_engine diag(&text_dia);
  lorina::return_code parser_ret;

  std::ifstream verilog_file(result["verilog"].as<std::string>(),
                             std::ifstream::in);
  if (!verilog_file.is_open()) {
    ERROR("Couldn't open verilog file")
    return 2;
  }

  parser_ret = lorina::read_genlib(result["genlib"].as<std::string>(), io);
  if (parser_ret != lorina::return_code::success) {
    ERROR("FATAL: Couldn't parse genlib file");
    return 2;
  }

  lorina::verilog_parser custom_parser(verilog_file, io, &diag);
  io.transfer_gates(custom_parser);

  parser_ret = lorina::read_verilog_custom_execute(custom_parser);
  if (parser_ret != lorina::return_code::success) {
    ERROR("FATAL: Couldn't parse verilog file");
    return 2;
  }

  if (!data.find_initial_placement()) {
    panic("Couldn't find an initial placement. Try increasing the chip "
          "area.");
  }

  // 3. annealing
  [[maybe_unused]]
  uint64_t initial_cost = hpwl(data);
  [[maybe_unused]]
  uint64_t final_cost;

  LOG_INFO("Initial cost: ", initial_cost);
  if (steps == 0) {
    LOG_INFO("No annealing steps")
    return 0;
  }

  logger.file_prefix.append("_initial");
  save_pgm(data, logger);
  logger.file_prefix = result["log_file"].as<std::string>();

  final_cost = anneal(data, initial_temp, final_temp, initial_window_x,
                      final_window_x, initial_window_y, final_window_y, steps,
                      warmup_steps, tuning_steps, initial_moves_per_step,
                      final_moves_per_step, logging_enabled, logger);
  // 4. present results
  logger.file_prefix.append("_final");
  save_pgm(data, logger);
  LOG_INFO("Final cost: ", final_cost);
  return 0;
}
