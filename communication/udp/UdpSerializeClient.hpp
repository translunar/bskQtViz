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
// UdpSerializeClient.h
//
// University of Colorado, Autonomous Vehicle Systems (AVS) Lab
// Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
//

#ifndef UDP_SERIALIZE_CLIENT_H
#define UDP_SERIALIZE_CLIENT_H

#include "UdpClient.hpp"

#include <iostream>
#include <sstream>
#include <boost/archive/text_iarchive.hpp>

template <typename T>
class UdpSerializeClient
    : public UdpClient
{
public:
    UdpSerializeClient(boost::asio::io_service *ioService);

    int getData(T *data);
};

template <typename T>
UdpSerializeClient::UdpSerializeClient(boost::asio::io_service *ioService)
    : UdpClient(ioService)
{
}

template <typename T>
int UdpSerializeClient::receiveSpacecraftSimData(T *data)
{
    m_stream->get_io_service().poll()
    if(m_inboundBuffer.empty()) {
        std::cout << "Received nothing" << std::endl;
    } else {
        std::string s(&m_inboundBuffer[0]);
        std::istringstream archiveStream(s, std::istringstream::binary | std::istringstream::in);
        try {
            boost::archive::text_iarchive archive(archiveStream);
            archive >> *data;
        } catch(boost::archive::archive_exception e) {
            std::cout << "Error occured: " << e.what() << std::endl;
            return 1;
        }

        std::cout << "Received: " << archiveStream.str() << std::endl;
    }
    return 0;
}

#endif