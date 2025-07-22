#include "../include/panic.h"
#include <cstdlib>

[[noreturn]]
void panic(std::string reason, const std::source_location location) {
  ERROR("FATAL: File ", location.file_name(), " Line ", location.line(),
        " Function ", location.function_name(), " with reason: ", reason)
  exit(1);
}
