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
#include "zeno/net/header.hpp"

using boost::asio::ip::udp;

constexpr static int kMaxLength = 1024;
constexpr static int kMsgLength = 64;
constexpr static int kClientSendBatch = 100;

std::atomic<uint64_t> count{0};

void client_loop(int id, const char *host, const char *port)
{
    char buffer[kMaxLength];
    char dev_null[kMaxLength];

    auto &packet_header = *(zeno::net::PacketHeader *) buffer;

    packet_header.packet_length = kMsgLength;
    packet_header.client_id = id;
    packet_header.packet_type = zeno::net::PacketType::Normal;

    boost::asio::io_context io_context;

    udp::socket s(io_context, udp::endpoint(udp::v4(), 0));

    udp::resolver resolver(io_context);
    udp::resolver::results_type endpoints =
        resolver.resolve(udp::v4(), host, port);

    info("Client %d connects to %s:%s", id, host, port);

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
    check(kMsgLength < kMaxLength, "msg size should < max length");

    std::vector<std::thread> client_threads;

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

    if (argc != 4)
    {
        std::cerr << "Usage: blocking_udp_echo_client <host> <port> <thread>\n";
        return 1;
    }
    const char *host = argv[1];
    const char *port = argv[2];
    int thread_nr = std::stoi(argv[3]);

    for (int i = 0; i < thread_nr; ++i)
    {
        client_threads.emplace_back(client_loop, i, host, port);
    }

    try
    {
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