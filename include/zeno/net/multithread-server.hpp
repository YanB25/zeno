#ifndef NET_MULTITHREAD_SERVER_H
#define NET_MULTITHREAD_SERVER_H

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <unordered_map>

#include "zeno/debug.hpp"
#include "zeno/net/header.hpp"

namespace zeno
{
namespace net
{
using boost::asio::ip::udp;
class MultithreadServer;

class UDPSession : public boost::enable_shared_from_this<UDPSession>
{
public:
    UDPSession(MultithreadServer *server) : server_(server)
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
    MultithreadServer *server_{nullptr};
    udp::endpoint remote_endpoint_;
    std::array<char, 2048> recv_buffer_;
    std::string message_;
};

class MultithreadServer
{
public:
    MultithreadServer(boost::asio::io_context &io_context, short port)
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
}  // namespace net
}  // namespace zeno
#endif