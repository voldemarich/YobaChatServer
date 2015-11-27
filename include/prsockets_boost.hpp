#include <cstdlib>
#include <iostream>
#include <vector>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "yobaprotocol.hpp"


using boost::asio::ip::tcp;

class session
{
public:
    session(boost::asio::io_service& io_service)
        : socket_(io_service)
    {
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
    if(socket_.available() < 1){
        async_read_until(socket_, data_,
                  stopseq,
                                boost::bind(&session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
        }
    }

private:
    void handle_read(const boost::system::error_code& error,
                     size_t bytes_transferred)
    {
        if (!error)
        {
            string request = streamBufferToString(&data_);
            if(request.length()>0){
                string responce = getResponce(request);
                readStringToStreamBuffer(&responce, &data_);
            }
            boost::asio::async_write(socket_, data_,
                                     boost::bind(&session::handle_write, this,
                                                 boost::asio::placeholders::error));

        }
        else
        {
            socket_.close();
            delete this;
        }
    }

    void handle_write(const boost::system::error_code& error)
    {
        if (!error)
        {
            async_read_until(socket_, data_,
                  stopseq,
                                boost::bind(&session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            socket_.close();
            delete this;
        }
    }

    tcp::socket socket_;
    boost::asio::streambuf data_;
};

class server
{
public:
    server(boost::asio::io_service& io_service, short port)
        : io_service_(io_service),
          acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        start_accept();
    }

private:
    void start_accept()
    {
        session* new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&server::handle_accept, this, new_session,
                                           boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session,
                       const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
        }
        else
        {
            delete new_session;
        }

        start_accept();
    }

    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

int initsrv()
{
    try
    {
        boost::asio::io_service io_service;

        server s(io_service, 4444);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
};
