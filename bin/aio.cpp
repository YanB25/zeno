#ifndef AIO_H
#define AIO_H

#include <inttypes.h>
#include <libaio.h>

#include <atomic>
#include <thread>
#include <vector>

#include "zeno/define.hpp"

namespace ybench
{
namespace aio
{
/**
 * @brief Benchmarking uint64_t
 */
class Logger
{
public:
    static constexpr size_t kBlockSize = 4 * define::KiB;
    static constexpr size_t kMaxEvent = 1024;
    static constexpr size_t kAIOAlignment = 512;

    Logger(std::string filename);
    ~Logger();
    /**
     * @brief run the benchmark
     *
     * @param threads number of threads to run the benchmark
     * @param seconds duration of the benchmark
     *
     */
    void Run(int threads, int size, int seconds);

private:
    std::atomic<uint64_t> count_{0};
    std::atomic<bool> stop_{false};
    std::vector<std::thread> threads_;
    int fd_{-1};
    io_context_t ctx_;
    std::atomic<size_t> offset_{0};
};

class InPlaceWrite
{
public:
    static constexpr size_t kBlockSize = 4 * define::KiB;
    static constexpr size_t kMaxEvent = 1024;
    static constexpr size_t kAIOAlignment = 512;

    InPlaceWrite(std::string filename);
    ~InPlaceWrite();
    /**
     * @brief run the benchmark
     *
     * @param threads number of threads to run the benchmark
     * @param seconds duration of the benchmark
     *
     */
    void Run(int threads, int size, int seconds);

private:
    std::atomic<uint64_t> count_{0};
    std::atomic<bool> stop_{false};
    std::vector<std::thread> threads_;
    int fd_{-1};
    io_context_t ctx_;
};

}  // namespace aio
}  // namespace ybench

#endif