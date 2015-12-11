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
    session(boost::asio::io_service& io_service, DbService * dbcon)
        : socket_(io_service)
    {
        this->dbcon = dbcon;
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        if(socket_.available() < 1)
        {
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
            if(request.length()>0)
            {
                string responce = getResponce(request, dbcon);
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
    DbService * dbcon;
};

class server
{
public:
    server(boost::asio::io_service& io_service, short port, DbService * dbcon)
        : io_service_(io_service),
          acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        this->dbcon = dbcon;
        start_accept();
    }

private:
    void start_accept()
    {
        session* new_session = new session(io_service_, dbcon);
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
    DbService * dbcon;
};

int initsrv(int serverport, std::string dbhost, std::string dbport, std::string dbname, std::string dbuser, std::string dbpassword)
{
    try
    {
        boost::asio::io_service io_service;
        DbService * dbserv = new DbService("tcp://"+dbhost+":"+dbport, dbuser, dbpassword, dbname);
        server s(io_service, 4444, dbserv);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "YobaChatServer Exception: " << e.what() << "\n";
    }
    return 0;
};
