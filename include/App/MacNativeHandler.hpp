#pragma once

#include <vector>
#include <string>

extern "C" void MakeWindowKey(void* windowPtr);
extern "C" void DeactivateApp();
extern "C++" std::vector<std::string> spotlightSearch(const std::string& queryString);
