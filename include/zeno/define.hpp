#ifndef DEFINE_H
#define DEFINE_H
#include <stdint.h>
namespace zeno
{
namespace define
{
constexpr uint64_t KiB = 1024ull;
constexpr uint64_t MiB = 1024ull * KiB;
constexpr uint64_t GiB = 1024ull * MiB;
constexpr uint64_t TiB = 1024ull * GiB;

constexpr uint64_t K = 1000ull;
constexpr uint64_t M = 1000ull * K;
constexpr uint64_t G = 1000ull * M;
constexpr uint64_t T = 1000ull * G;
}  // namespace define
}  // namespace zeno
#endif