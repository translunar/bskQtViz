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
// UdpClient.cpp
//
// University of Colorado, Autonomous Vehicle Systems (AVS) Lab
// Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
//

#include "UdpClient.hpp"

UdpClient::UdpClient(boost::asio::io_service *ioService)
{
    m_stream.reset(new boost::asio::ip::udp::socket(*ioService));
}

int UdpClient::connect(std::string multicastAddress, std::string portNum, std::string ipAddress)
{
    boost::system::error_code ec;
    boost::asio::ip::udp::resolver resolver(m_stream->get_io_service());
    boost::asio::ip::udp::resolver::query query(ipAddress, portNum);
    boost::asio::ip::udp::endpoint listenEndpoint = *resolver.resolve(query, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    m_stream->open(listenEndpoint.protocol(), ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    m_stream->set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    m_stream->set_option(boost::asio::ip::multicast::enable_loopback(true), ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    m_stream->bind(listenEndpoint, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }
    m_stream->set_option(boost::asio::ip::multicast::join_group(
                             boost::asio::ip::address::from_string(multicastAddress).to_v4(),
                             boost::asio::ip::address::from_string(ipAddress).to_v4()), ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    m_stream->async_receive_from(
        boost::asio::buffer(m_inboundBuffer),
        m_senderEndpoint,
        boost::bind(&UdpClient::handleAsyncReceiveFrom, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));

    return 0;
}

bool UdpClient::receiveData(std::vector<char> &data)
{
    m_stream->get_io_service().poll();
    data = m_inboundBuffer;
    return true;
}

void UdpClient::handleAsyncReceiveFrom(const boost::system::error_code &ec, size_t bytesReceived)
{
    if(!ec) {
        m_stream->async_receive_from(
            boost::asio::buffer(m_inboundBuffer),
            m_senderEndpoint,
            boost::bind(&UdpClient::handleAsyncReceiveFrom, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    } else {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
    }
}
