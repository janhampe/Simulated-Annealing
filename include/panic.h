#pragma once

#include <string>
#include <source_location>

[[noreturn]]
void panic(std::string reason, const std::source_location location = std::source_location::current());

