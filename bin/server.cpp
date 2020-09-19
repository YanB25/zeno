//
// async_udp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "zeno/debug.hpp"
#include "zeno/parser.hpp"

using boost::asio::ip::udp;

class server
{
public:
    server(boost::asio::io_context &io_context, short port)
        : socket_(io_context, udp::endpoint(udp::v4(), port)),
          deadline_(io_context)
    {
        info("Server is listening on 0.0.0.0:%d", port);
        deadline_.expires_from_now(boost::posix_time::seconds(1));

        check_timeout();
        do_receive();
    }

    void check_timeout()
    {
        if (deadline_.expires_at() <=
            boost::asio::deadline_timer::traits_type::now())
        {
            dinfo("Heartbeat one second.");
            deadline_.expires_from_now(boost::posix_time::seconds(1));
        }
        deadline_.async_wait([&](boost::system::error_code ec) {
            if (!ec)
            {
                check_timeout();
            }
            else
            {
                error("timer get errno: %d", ec.value());
            }
        });
    }

    void do_receive()
    {
        socket_.async_receive_from(
            boost::asio::buffer(data_, max_length),
            sender_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0)
                {
                    dinfo("server recv msg with size = %lu", bytes_recvd);

                    int client_id = zeno::parse::ParseClientId(data_);
                    if (endpoint_map_.find(client_id) == endpoint_map_.end())
                    {
                        info("Permanently add (%d, %s:%d) into known clients",
                             client_id,
                             sender_endpoint_.address().to_string().c_str(),
                             sender_endpoint_.port());
                        endpoint_map_[client_id] = sender_endpoint_;
                    }
                    do_send(bytes_recvd);
                }
                else
                {
                    do_receive();
                }
            });
    }

    void do_send(std::size_t length)
    {
        socket_.async_send_to(
            boost::asio::buffer(data_, length),
            sender_endpoint_,
            [this](boost::system::error_code /*ec*/,
                   std::size_t /*bytes_sent*/) { do_receive(); });
    }

private:
    udp::socket socket_;
    udp::endpoint sender_endpoint_;
    std::unordered_map<int, udp::endpoint> endpoint_map_;

    boost::asio::deadline_timer deadline_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];
};

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: async_udp_echo_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;

        server s(io_context, std::atoi(argv[1]));

        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}