#pragma once

#include <string>
#include <vector>

extern "C" void deactivateApp();

extern "C++" std::vector<std::string> spotlightSearch(const std::string& query,
                                                      const std::vector<std::string>& folders);
