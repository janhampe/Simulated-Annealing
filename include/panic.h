#pragma once

#include <string>
#include <iostream>
#include <source_location>
#include "debug.h"

[[noreturn]]
void panic(std::string reason, const std::source_location location = std::source_location::current());

