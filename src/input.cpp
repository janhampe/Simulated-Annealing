#include "../include/input.h"
#include <cstdint>
#include <utility>

IO::IO(Data &data) : data(data) {
  block_num = 0;
  net_num = 0;
}

const std::pair<uint32_t, uint32_t> IO::find_best_sides(double area) const {
  if (area <= 1.0) {
    LOG_INFO(
        "Gate with area below or equal to 1.0 was rounded to 1.0. True area "
        "was: ",
        area);
    return std::make_pair(1, 1);
  }
  uint32_t best_a = 1;
  uint32_t best_b = 1;
  double best_error = std::numeric_limits<double>::max();

  int max_a = static_cast<uint32_t>(std::ceil(std::sqrt(area)));

  for (int a = 1; a <= max_a; a++) {
    double ideal_b = area / a;

    // Try floor and ceil of ideal_b
    for (int b : {static_cast<uint32_t>(std::floor(ideal_b)),
                  static_cast<uint32_t>(std::ceil(ideal_b))}) {
      if (b < a || b > 3 * a)
        continue;

      double error = std::abs(static_cast<double>(a) * b - area);
      if (error < best_error) {
        best_error = error;
        best_a = a;
        best_b = b;
      }
    }
  }

  return {best_a, best_b};
}

void IO::on_gate(std::string const &name, std::string const &expression,
                 double area, std::vector<lorina::pin_spec> const &pins,
                 std::string const &output_pin) const {
  DEBUG("New gate: ", name)
  auto [y, x] = find_best_sides(area);
  genlib_gate gate = {name, area, x, y, static_cast<uint32_t>(pins.size())};
  gate_types.emplace(std::make_pair(name, gate));
}

void IO::on_inputs(const std::vector<std::string> &inputs,
                   std::string const &size) const {
  DEBUG("Reading inputs")
  for (auto in : inputs) {
    DEBUG("Input ", net_num, " name: ", in)
    net_name_to_id.emplace(std::make_pair(in, net_num));
    data.add_net({.id = net_num, .pins = {}});
    net_num++;
  }
}

void IO::on_outputs(const std::vector<std::string> &outputs,
                    std::string const &size) const {
  DEBUG("Reading outputs")
  for (auto out : outputs) {
    DEBUG("Output ", net_num, " name: ", out)
    net_name_to_id.emplace(std::make_pair(out, net_num));
    data.add_net({.id = net_num, .pins = {}});
    net_num++;
  }
}

void IO::on_wires(const std::vector<std::string> &wires,
                  std::string const &size) const {
  DEBUG("Reading wires")
  for (auto wire : wires) {
    DEBUG("Wire ", net_num, " name: ", wire)
    net_name_to_id.emplace(std::make_pair(wire, net_num));
    data.add_net({.id = net_num, .pins = {}});
    net_num++;
  }
}

void IO::on_module_instantiation(
    std::string const &module_name, std::vector<std::string> const &params,
    std::string const &inst_name,
    std::vector<std::pair<std::string, std::string>> const &args) const {

  DEBUG("Instantiating module ", inst_name, " of type ", module_name)
  genlib_gate type = gate_types.at(module_name);
  uint64_t id = block_num;
  block_id_to_inst_name.insert(std::make_pair(id, inst_name));
  block_id_to_gate_type.insert(std::make_pair(id, type.name));

  block b = {.id = id,
             .x = 0,
             .y = 0,
             .len_x = type.len_x,
             .len_y = type.len_y,
             .net_ids = {}};
  for (auto [mod_sig, inst_sig] : args) {
    uint64_t net_id = net_name_to_id.at(inst_sig);
    b.net_ids.emplace_back(net_id);
  }

  data.add_block(b);
  block_num++;
}
