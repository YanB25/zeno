//
// blocking_udp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <inttypes.h>

#include <atomic>
#include <boost/asio.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <zeno/smart.hpp>

#include "zeno/debug.hpp"

using boost::asio::ip::udp;

constexpr static int kMaxLength = 1024;
constexpr static int kMsgLength = 64;
constexpr static int kClientSendBatch = 100;
constexpr static int kClientThread = 14;

std::atomic<uint64_t> count{0};

void client_loop(int argc, char *argv[], int id)
{
    char buffer[kMaxLength];
    char dev_null[kMaxLength];

    if (argc != 3)
    {
        panic("Usage: blocking_udp_echo_client <host> <port>");
    }

    boost::asio::io_context io_context;

    udp::socket s(io_context, udp::endpoint(udp::v4(), 0));

    udp::resolver resolver(io_context);
    udp::resolver::results_type endpoints =
        resolver.resolve(udp::v4(), argv[1], argv[2]);

    info("Client %d connects to %s:%s", id, argv[1], argv[2]);

    udp::endpoint sender_endpoint;

    while (true)
    {
        for (int i = 0; i < kClientSendBatch; ++i)
        {
            s.send_to(boost::asio::buffer(buffer, kMsgLength),
                      *endpoints.begin());

            s.receive_from(boost::asio::buffer(dev_null, kMaxLength),
                           sender_endpoint);
        }
        count.fetch_add(kClientSendBatch, std::memory_order_relaxed);
    }
}

int main(int argc, char *argv[])
{
    char buffer[kMaxLength];

    check(kMsgLength < kMaxLength, "msg size should < max length");

    std::vector<std::thread> client_threads;

    for (int i = 0; i < kMsgLength; ++i)
    {
        buffer[i] = 'a';
    }

    std::thread timer([&]() {
        uint64_t last_value = 0;
        auto last_time = std::chrono::steady_clock::now();
        while (true)
        {
            sleep(1);

            uint64_t value = count.load(std::memory_order_relaxed);
            auto now = std::chrono::steady_clock::now();

            uint64_t diff_value = value - last_value;
            auto diff_us =
                std::chrono::duration_cast<std::chrono::microseconds>(now -
                                                                      last_time)
                    .count();

            double ops = diff_value * 1000 * 1000 / diff_us;

            info("Client Ops: %s (diff_value: %" PRIu64 " in diff_us: %" PRIu64
                 " )",
                 zeno::smart::toOps(ops).c_str(),
                 diff_value,
                 diff_us);

            last_value = value;
            last_time = now;
        }
    });

    for (int i = 0; i < kClientThread; ++i)
    {
        client_threads.emplace_back(client_loop, argc, argv, i);
    }

    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: blocking_udp_echo_client <host> <port>\n";
            return 1;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    timer.join();
    for (auto &t : client_threads)
    {
        t.join();
    }

    return 0;
}