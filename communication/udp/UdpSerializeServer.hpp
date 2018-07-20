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
// UdpSerializeServer.h
//
// University of Colorado, Autonomous Vehicle Systems (AVS) Lab
// Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
//

#ifndef UDP_SERIALIZE_SERVER_H
#define UDP_SERIALIZE_SERVER_H

#include "UdpServer.hpp"

#include <iostream>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>

template <typename T>
class UdpSerializeServer
    : public UdpServer
{
public:
    UdpSerializeServer(boost::asio::io_service *ioService);

    int sendData(T *data);
};

template <typename T>
UdpSerializeServer<T>::UdpSerializeServer(boost::asio::io_service *ioService)
    : UdpServer(ioService)
{
}

template <typename T>
int UdpSerializeServer<T>::sendData(T *data)
{
    // Serialize the data
    std::ostringstream archiveStream;
    boost::archive::text_oarchive archive(archiveStream);
    archive << const_cast<const SpacecraftSim &>(*data);
    unsigned int dataSize = archiveStream.str().length();
    if(dataSize > STREAM_BUFFER_SIZE) {
        std::cout << "Error in " << __FUNCTION__ << ": data exceeds stream buffer size" << std::endl;
        return 1;
    }
    m_outboundBuffer = archiveStream.str();

    //std::cout << "Sending data" << std::endl;
    std::cout << "Sending: " << m_outboundBuffer << std::endl;

    setNewBuffer();
    asyncSendTo();
    return 0;
}

#endif