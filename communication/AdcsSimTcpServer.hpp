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
// AdcsSimTcpServer.h
//
// University of Colorado, Autonomous Vehicle Systems (AVS) Lab
// Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
//

#ifndef ADCS_SIM_IO_DEVICE_H
#define ADCS_SIM_IO_DEVICE_H

#include "tcpServer.hpp"

class SpacecraftSim;

typedef enum {
    DEVICE_RW1,
    DEVICE_RW2,
    DEVICE_RW3,
    DEVICE_RW4,
    DEVICE_ST,
    DEVICE_IRU,
    DEVICE_TAM,
    DEVICE_POD,
    DEVICE_CDH,
    MAX_SERVER_DEVICE
} ServerDevice_t;
std::string serverDevice2str(ServerDevice_t device);

class AdcsSimTcpServer : public TcpServer
{
public:
    AdcsSimTcpServer(boost::asio::io_service *ioService);
    virtual ~AdcsSimTcpServer();

    void checkForAsyncData(ServerDevice_t device, SpacecraftSim *scSim);

protected:
    char m_storedBuffer[STREAM_BUFFER_SIZE];
    size_t m_storedBufferSize;
    size_t m_storedBufferWrites;

    void handleCheckForAsyncData(const boost::system::error_code &ec,
                                 size_t bytes_transferred,
                                 ServerDevice_t device,
                                 SpacecraftSim *scSim);
};

#endif