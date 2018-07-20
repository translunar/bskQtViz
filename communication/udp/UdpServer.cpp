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
// UdpServer.h
//
// University of Colorado, Autonomous Vehicle Systems (AVS) Lab
// Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
//

#include "UdpServer.hpp"

UdpServer::UdpServer(boost::asio::io_service *ioService)
    : m_isNewBuffer(true)
{
    m_stream.reset(new boost::asio::ip::udp::socket(*ioService));
}

int UdpServer::startBroadcast(std::string multicastAddress, std::string portNum)
{
    boost::system::error_code ec;
    boost::asio::ip::udp::resolver resolver(m_stream->get_io_service());
    boost::asio::ip::udp::resolver::query query(multicastAddress, portNum);
    m_endpoint = *resolver.resolve(query, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    m_stream->open(m_endpoint.protocol(), ec);
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

    m_stream->async_send_to(
        boost::asio::buffer(m_outboundBuffer),
        m_endpoint,
        boost::bind(&UdpServer::handleAsyncSendTo, this,
                    boost::asio::placeholders::error));

    return 0;
}

bool UdpServer::sendData(std::string data)
{
    if(data.size() > STREAM_BUFFER_SIZE) {
        return false;
    }
    m_outboundBuffer = data;
    setNewBuffer();
    asyncSendTo();
    return true;
}

void UdpServer::asyncSendTo()
{
    m_stream->async_send_to(
        boost::asio::buffer(m_outboundBuffer),
        m_endpoint,
        boost::bind(&UdpServer::handleAsyncSendTo, this,
                    boost::asio::placeholders::error));
}

void UdpServer::handleAsyncSendTo(const boost::system::error_code &ec)
{
    if(!ec) {
        if(m_isNewBuffer) {
            asyncSendTo();
            m_isNewBuffer = false;
        }
    } else {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
    }
}