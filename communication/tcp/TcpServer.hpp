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
// TcpServer.h
//
// University of Colorado, Autonomous Vehicle Systems (AVS) Lab
// Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
//

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "basicIoDevice.hpp"

class TcpServer
    : public BasicIoObject_t<boost::asio::ip::tcp::socket>
{
public:
    TcpServer(boost::asio::io_service *ioService);

    bool acceptConnections(std::string ipAddress = "127.0.0.1",
                           std::string portNum = "50000");
    virtual bool close(void);

    virtual bool receiveData(std::vector<char> &data);
    virtual bool sendData(std::string data);

    virtual void clearBuffers(void);

private:
    boost::scoped_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
};

#endif