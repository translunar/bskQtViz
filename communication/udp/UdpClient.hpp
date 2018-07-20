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
// UdpClient.h
//
// University of Colorado, Autonomous Vehicle Systems (AVS) Lab
// Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
//

#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include "BasicIoDevice.hpp"

class UdpClient
    : public BasicIoObject_t<boost::asio::ip::udp::socket>
{
public:
    UdpClient(boost::asio::io_service *ioService);

    /* Default ipAddress shouldn't have to be changd, as this address should pick any available address */
    int connect(std::string multicastAddress = "239.255.0.1",
                std::string portNum = "50000",
                std::string ipAddress = "0.0.0.0");

    virtual bool receiveData(std::vector<char> &data);

private:
    virtual void handleAsyncReceiveFrom(const boost::system::error_code &error, size_t bytesReceived);
    boost::asio::ip::udp::endpoint m_senderEndpoint;

};

#endif
