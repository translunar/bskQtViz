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

#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include "basicIoDevice.hpp"

class UdpServer
    : public BasicIoObject_t<boost::asio::ip::udp::socket>
{
public:
    UdpServer(boost::asio::io_service *ioService);

    int startBroadcast(std::string multicastAddress = "239.255.0.1",
                       std::string portNum = "50000");

    virtual bool sendData(std::string data);

protected:
    void setNewBuffer() {
        m_isNewBuffer = true;
    }
    void asyncSendTo();

private:
    void handleAsyncSendTo(const boost::system::error_code &error);
    boost::asio::ip::udp::endpoint m_endpoint;
    bool m_isNewBuffer;
};

#endif