#include "../include/panic.h"
#include <cstdlib>

[[noreturn]]
void panic(Data& d, std::string reason, const std::source_location location) {
  std::cout << "FATAL: File " << location.file_name() << " Line "
            << location.line() << " Function " << location.function_name()
            << " with reason: " << reason << std::endl;
  exit(1);
}
