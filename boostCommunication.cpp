/*
 ISC License

 Copyright (c) 2016-2017, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */
//
// boost_communication.cpp
//
// University of Colorado, Autonomous Vehicle Systems (AVS) Lab
// Unpublished Copyright (c) 2012-2013 University of Colorado, All Rights Reserved
//

#include "boostCommunication.h"

#include <ctime>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

int TCPIP_OpenGL(ComStatus_t comStatus, unsigned char *message, size_t sendSize, size_t *inputSize)
{
    static boost::shared_ptr<boost::asio::io_service> io_service(new boost::asio::io_service);
    static TcpClient client(io_service);

    switch(comStatus) {
        case COM_INIT:
            /* Initialize the TCP/IP communication */
            if(client.connect(_ip_address, TCPIP_PORT) != 0) {
                std::cout << "Error in " << __FUNCTION__ << std::endl;
                return 1;
            }
            break;

        case COM_SHUTDOWN:
            io_service->stop();
            /* close down the TCP/IP communication */
            if(client.close() != 0) {
                std::cout << "Error in " << __FUNCTION__ << std::endl;
                return 1;
            }
            break;

        case COM_SEND:
            if(client.sendData(message, sendSize) != 0) {
                std::cout << "called by " << __FUNCTION__ << std::endl;
                return 1;
            }
            break;

        case COM_RECEIVE:
            if(client.receiveData(message, inputSize) != 0) {
                std::cout << "called by " << __FUNCTION__ << std::endl;
                return 1;
            }
            break;

        default:
            std::cout << "Error: " << comStatus << " is not a valid TCPIP status" << std::endl;
            return 1;
            break;
    }

    return 0;
}


TcpServer::TcpServer(boost::shared_ptr<boost::asio::io_service> io_service)
    : BasicIOObject_t(io_service)
    , acceptor_(new boost::asio::ip::tcp::acceptor(*io_service))
{
    stream_.reset(new boost::asio::ip::tcp::socket(*io_service));
}

int TcpServer::startListening(std::string ipAddress, std::string portNum)
{
    boost::system::error_code ec;
    boost::asio::ip::tcp::resolver resolver(*io_service_);
    boost::asio::ip::tcp::resolver::query query(ipAddress, portNum);
    endpoint_ = *resolver.resolve(query, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    acceptor_->open(endpoint_.protocol(), ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    acceptor_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(false), ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    acceptor_->bind(endpoint_, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    acceptor_->listen(boost::asio::socket_base::max_connections, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    return 0;
}

int TcpServer::acceptConnection(void)
{
    boost::system::error_code ec;
    acceptor_->accept(*stream_, endpoint_, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    return 0;
}

int TcpServer::close(void)
{
    boost::system::error_code ec;
    acceptor_->close(ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    stream_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    stream_->close(ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    return 0;
}

TcpClient::TcpClient(boost::shared_ptr<boost::asio::io_service> io_service)
    : BasicIOObject_t(io_service)
{
    stream_.reset(new boost::asio::ip::tcp::socket(*io_service));
}

int TcpClient::connect(std::string ipAddress, std::string portNum)
{
    boost::system::error_code ec;
    boost::asio::ip::tcp::resolver resolver(*io_service_);
    boost::asio::ip::tcp::resolver::query query(ipAddress, portNum);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    for(int attempCount = 0; attempCount < MAX_CONNECT_ATTEMPTS; attempCount++) {
        ec.clear();
        stream_->connect(endpoint, ec);
        if(ec) {
            std::cout << "Connection attempt " << attempCount << " failed";
            if(attempCount == MAX_CONNECT_ATTEMPTS - 1) {
                std::cout << std::endl;
                std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
                return 1;
            } else {
                // Wait before attempting another connection
                std::cout << ", retrying in ";
                for(int i = 3; i > 0; i--) {
                    time_t start = time(NULL);
                    while(difftime(time(NULL), start) < 1.0);
                    std::cout << i << "...";
                    std::cout.flush();
                }
                std::cout << std::endl;
            }
        } else {
            break;
        }
    }
    // If we still can't connect then abort
    if(ec) {
        return 1;
    }
    return 0;
}

SerialConnection::SerialConnection(boost::shared_ptr<boost::asio::io_service> io_service)
    : BasicIOObject_t(io_service)
{
    stream_.reset(new boost::asio::serial_port(*io_service));
}

int SerialConnection::connect(std::string deviceName,
                              unsigned int baudRate,
                              boost::asio::serial_port_base::character_size optCharSize,
                              boost::asio::serial_port_base::flow_control optFlowControl,
                              boost::asio::serial_port_base::parity optParity,
                              boost::asio::serial_port_base::stop_bits optStopBits)
{
    if(isOpen()) {
        if(close() != 0) {
            std::cout << "called by " << __FUNCTION__ << std::endl;
            return 1;
        }
    }

    boost::system::error_code ec;
    stream_->open(deviceName, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    stream_->set_option(boost::asio::serial_port_base::baud_rate(baudRate), ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    stream_->set_option(optCharSize, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    stream_->set_option(optFlowControl, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    stream_->set_option(optParity, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    stream_->set_option(optStopBits, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    return 0;
}

