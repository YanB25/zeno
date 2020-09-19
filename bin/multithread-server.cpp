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
#include <boost/asio/strand.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "zeno/debug.hpp"
#include "zeno/parser.hpp"

using boost::asio::ip::udp;
class server;

class UDPSession : public boost::enable_shared_from_this<UDPSession>
{
public:
    UDPSession(server *server) : server_(server)
    {
    }
    void handle_request(const boost::system::error_code &ec);

    void handle_sent(const boost::system::error_code &ec, std::size_t)
    {
        if (ec)
        {
            error("Error sending response");
        }
    }
    udp::endpoint &remote_endpoint()
    {
        return remote_endpoint_;
    }
    std::array<char, 2048> &buffer()
    {
        return recv_buffer_;
    }
    std::string message()
    {
        return message_;
    }

private:
    server *server_{nullptr};
    udp::endpoint remote_endpoint_;
    std::array<char, 2048> recv_buffer_;
    std::string message_;
};

class server
{
public:
    server(boost::asio::io_context &io_context, short port)
        : socket_(io_context, udp::endpoint(udp::v4(), port)),
          strand_(io_context),
          deadline_(io_context)
    {
        info("Server is listening on 0.0.0.0:%d", port);
        deadline_.expires_from_now(boost::posix_time::seconds(1));

        check_timeout();
        receive_session();
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

    void receive_session()
    {
        auto session = boost::make_shared<UDPSession>(this);

        socket_.async_receive_from(
            boost::asio::buffer(session->buffer()),
            session->remote_endpoint(),
            strand_.wrap([this, session](boost::system::error_code ec,
                                         std::size_t bytes_recvd) {
                handle_receive(session, ec, bytes_recvd);
            }));
    }

    void enqueue_response(const boost::shared_ptr<UDPSession> &session)
    {
        socket_.async_send_to(
            boost::asio::buffer(session->message()),
            session->remote_endpoint(),
            strand_.wrap([&](const boost::system::error_code &ec,
                             std::size_t recv_bytes) {
                session->handle_sent(ec, recv_bytes);
            }));
    }

    void handle_receive(boost::shared_ptr<UDPSession> session,
                        const boost::system::error_code &ec,
                        std::size_t)
    {
        boost::asio::post(socket_.get_executor(),
                          [&ec, session]() { session->handle_request(ec); });
        receive_session();
    }

    void do_send(std::size_t length)
    {
        socket_.async_send_to(
            boost::asio::buffer(data_, length),
            sender_endpoint_,
            [this](boost::system::error_code /*ec*/,
                   std::size_t /*bytes_sent*/) { receive_session(); });
    }

private:
    udp::socket socket_;
    udp::endpoint sender_endpoint_;
    std::unordered_map<zeno::net::ClientId, udp::endpoint> endpoint_map_;

    boost::asio::io_service::strand strand_;

    boost::asio::deadline_timer deadline_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];
};

void UDPSession::handle_request(const boost::system::error_code &ec)

{
    if (!ec || ec == boost::asio::error::message_size)
    {
        server_->enqueue_response(shared_from_this());
    }
}

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

        server s(io_context, std::atoi(argv[1]));
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