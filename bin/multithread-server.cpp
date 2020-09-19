//
// async_udp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "zeno/net/multithread-server.hpp"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "zeno/debug.hpp"
#include "zeno/net/parser.hpp"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: async_udp_echo_server <port> <thread>\n";
            return 1;
        }

        boost::asio::io_context io_context;

        zeno::net::MultithreadServer s(io_context, std::atoi(argv[1]));
        size_t thread_nr = std::stoi(argv[2]);

        boost::thread_group tg;
        for (size_t i = 0; i < thread_nr; ++i)
        {
            tg.create_thread([&]() { io_context.run(); });
        }

        tg.join_all();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}