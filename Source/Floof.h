#pragma once

#include <iostream>
#include <format>

namespace FLOOF {
#define LOG(msg, ...) std::cout << std::format(msg, __VA_ARGS__)
#ifndef NDEBUG
#define ASSERT(expr, msg, ...) do { if (!(expr)) { LOG(msg, __VA_ARGS__); __debugbreak(); } } while (false)
#else
#define ASSERT(expr, msg, ...)
#endif // !NDEBUG
}

