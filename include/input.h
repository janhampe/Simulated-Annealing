#include "data.h"
#include "debug.h"
#include "lorina/diagnostics.hpp"
#include "lorina/genlib.hpp"
#include "lorina/verilog.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct genlib_gate {
  std::string name;
  double area;
  uint32_t len_x;
  uint32_t len_y;
  uint32_t pin_num;
  std::string output_pin_name;
  std::vector<std::string> pin_names;
};

// TODO:
// Give pins different positions on block (Maybe read from lef file)
// Add output and input pins to the edge of the chip and allow moving them

class IO : public lorina::genlib_reader, public lorina::verilog_reader {
public:
  IO(Data &data);

  // For parsing genlib files
  void on_gate(std::string const &name, std::string const &expression,
               double area, std::vector<lorina::pin_spec> const &pins,
               std::string const &output_pin) const override;

  bool transfer_gates(lorina::verilog_parser &parser);

  // For parsing verilog files
  void on_inputs(const std::vector<std::string> &inputs,
                 std::string const &size = "") const override;
  void on_outputs(const std::vector<std::string> &outputs,
                  std::string const &size = "") const override;
  void on_wires(const std::vector<std::string> &wires,
                std::string const &size = "") const override;
  void on_module_instantiation(
      std::string const &module_name, std::vector<std::string> const &params,
      std::string const &inst_name,
      std::vector<std::pair<std::string, std::string>> const &args)
      const override;

private:
  Data &data;
  mutable uint64_t block_num;
  mutable uint64_t net_num;
  mutable std::unordered_map<std::string, genlib_gate> gate_types;
  mutable std::unordered_map<uint64_t, std::string> block_id_to_inst_name;
  mutable std::unordered_map<uint64_t, genlib_gate> block_id_to_gate_type;
  mutable std::unordered_map<std::string, uint64_t> net_name_to_id;

  // Find sides a and b where a <= b <= 3a
  const std::pair<uint32_t, uint32_t> find_best_sides(double area) const;
};

class lorina_diag : public lorina::diagnostic_consumer {
  void handle_diagnostic(lorina::diagnostic_level level,
                         std::string const &message) const {
    DEBUG("Lorina Diagnostic: ", message);
  }
};
