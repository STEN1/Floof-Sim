#pragma once

#include <iostream>
#include <string>

namespace FLOOF {
#define LOG(msg) std::cout << msg
#ifndef NDEBUG
#define ASSERT(expr, msg) do { if (!(expr)) { LOG(msg); __debugbreak(); } } while (false)
#else
#define ASSERT(expr, msg, ...)
#endif // !NDEBUG
}

