#include "zeno/disk/logger.hpp"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "zeno/common.hpp"
#include "zeno/debug.hpp"
#include "zeno/smart.hpp"
namespace zeno
{
namespace disk
{
Logger::Logger(std::string filename)
{
    fd_ = open(filename.c_str(), O_RDWR | O_DIRECT);
    check(fd_ >= 0, "failed to open file %s. Is it exists?", filename.c_str());

    memset(&ctx_, 0, sizeof(ctx_));
    check(io_setup(kMaxEvent, &ctx_) == 0, "io_setup error.");

    if (filename.find("/dev") == std::string::npos)
    {
        warn(
            "It seems that %s is not a device, but a file on a filesystem. "
            "libaio may not work well.",
            filename.c_str());
    }
}
Logger::~Logger()
{
    close(fd_);
    io_destroy(ctx_);
}
void Logger::Run(int threads, int size, int seconds)
{
    check((unsigned long) size >= kAIOAlignment,
          "size must be a multiple of KIOAlignment(%s), get %s",
          smart::toSize(kAIOAlignment).c_str(),
          smart::toSize(size).c_str());
    check((unsigned long) size % kAIOAlignment == 0,
          "size must be a multiple of KIOAlignment(%s), get %s",
          smart::toSize(kAIOAlignment).c_str(),
          smart::toSize(size).c_str());

    std::atomic<uint64_t> fail_count{0};
    std::atomic<unsigned long> fail_reason{0};

    for (int i = 0; i < threads; ++i)
    {
        threads_.emplace_back([&, i]() {
            char *msg;
            check(posix_memalign((void **) &msg, kAIOAlignment, size) == 0);
            check(msg != nullptr, "failed to alloc memory with alignment");

            iocb iocb_obj;
            io_event events[kMaxEvent];
            timespec timeout;
            timeout.tv_sec = 0;
            timeout.tv_nsec = 0;

            iocb *iocbs[1];
            iocbs[0] = &iocb_obj;

            while (!stop_.load(std::memory_order_relaxed))
            {
                long long offset = offset_.fetch_add(size);
                io_prep_pwrite(&iocb_obj, fd_, (void *) msg, size, offset);
                iocb_obj.data = (void *) (uint64_t) i;

                int ret = io_submit(ctx_, 1, iocbs);
                if (ret != -EAGAIN && ret < 0)
                {
                    check(
                        false, "io_submit pwrite failed with errno = %d", ret);
                }

                size_t num = io_getevents(ctx_, 0, kMaxEvent, events, &timeout);
                count_.fetch_add(num, std::memory_order_relaxed);

                for (size_t i = 0; i < num; ++i)
                {
                    dinfo("Write succeed for %" PRIu64,
                          (uint64_t) events[i].data);
                    if ((long) events[0].res <= 0)
                    {
                        fail_count.fetch_add(1, std::memory_order_relaxed);
                        fail_reason.store(-events[0].res,
                                          std::memory_order_relaxed);
                    }
                    break;
                }
            }
        });
    }

    // for (int i = 0; i < seconds; ++i)
    // {
    //     TimeGuard<uint64_t> tg("aio::Logger", [&]() {
    //         return count_.load(std::memory_order_relaxed);
    //     });
    //     sleep(1);
    // }

    if (fail_count != 0)
    {
        warn("Failed io_submit %" PRIu64 ", one of the reason is %lu",
             fail_count.load(),
             fail_reason.load());
    }

    stop_ = true;
    for (auto &t : threads_)
    {
        t.join();
    }
}

InPlaceWrite::InPlaceWrite(std::string filename)
{
    fd_ = open(filename.c_str(), O_RDWR | O_DIRECT);
    check(fd_ >= 0, "failed to open file %s. Is it exists?", filename.c_str());

    memset(&ctx_, 0, sizeof(ctx_));
    check(io_setup(kMaxEvent, &ctx_) == 0, "io_setup error.");

    if (filename.find("/dev") == std::string::npos)
    {
        warn(
            "It seems that %s is not a device, but a file on a filesystem. "
            "libaio may not work well.",
            filename.c_str());
    }
}
InPlaceWrite::~InPlaceWrite()
{
    close(fd_);
    io_destroy(ctx_);
}
void InPlaceWrite::Run(int threads, int size, int seconds)
{
    check((unsigned long) size >= kAIOAlignment,
          "size must be a multiple of KIOAlignment(%s), get %s",
          smart::toSize(kAIOAlignment).c_str(),
          smart::toSize(size).c_str());
    check((unsigned long) size % kAIOAlignment == 0,
          "size must be a multiple of KIOAlignment(%s), get %s",
          smart::toSize(kAIOAlignment).c_str(),
          smart::toSize(size).c_str());

    std::atomic<uint64_t> fail_count{0};
    std::atomic<unsigned long> fail_reason{0};

    for (int i = 0; i < threads; ++i)
    {
        threads_.emplace_back([&, i]() {
            char *msg;
            check(posix_memalign((void **) &msg, kAIOAlignment, size) == 0);
            check(msg != nullptr, "failed to alloc memory with alignment");

            iocb iocb_obj;
            io_event events[kMaxEvent];
            timespec timeout;
            timeout.tv_sec = 0;
            timeout.tv_nsec = 0;

            iocb *iocbs[1];
            iocbs[0] = &iocb_obj;

            while (!stop_.load(std::memory_order_relaxed))
            {
                io_prep_pwrite(
                    &iocb_obj, fd_, (void *) msg, size, 0 /* offset */);
                iocb_obj.data = (void *) (uint64_t) i;

                int ret = io_submit(ctx_, 1, iocbs);
                if (ret != -EAGAIN && ret < 0)
                {
                    check(
                        false, "io_submit pwrite failed with errno = %d", ret);
                }

                size_t num = io_getevents(ctx_, 0, kMaxEvent, events, &timeout);
                count_.fetch_add(num, std::memory_order_relaxed);

                for (size_t i = 0; i < num; ++i)
                {
                    dinfo("Write succeed for %" PRIu64,
                          (uint64_t) events[i].data);
                    if ((long) events[0].res <= 0)
                    {
                        fail_count.fetch_add(1, std::memory_order_relaxed);
                        fail_reason.store(-events[0].res,
                                          std::memory_order_relaxed);
                    }
                    break;
                }
            }
        });
    }

    if (fail_count != 0)
    {
        warn("Failed io_submit %" PRIu64 ", one of the reason is %lu",
             fail_count.load(),
             fail_reason.load());
    }

    stop_ = true;
    for (auto &t : threads_)
    {
        t.join();
    }
}

}  // namespace disk
}  // namespace zeno