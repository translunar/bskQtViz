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
// SerialConnection.h
//
// University of Colorado, Autonomous Vehicle Systems (AVS) Lab
// Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
//

#ifndef SERIAL_CONNECTION_H
#define SERIAL_CONNECTION_H

#include "BasicIoDevice.hpp"

#define SERIAL_BAUD_RATE 115200

class SerialConnection
    : public BasicIoObject_t<boost::asio::serial_port>
{
public:
    SerialConnection(boost::asio::io_service *ioService);

    int connect(std::string deviceName,
                unsigned int baudRate = SERIAL_BAUD_RATE,
                boost::asio::serial_port_base::character_size optCharSize = boost::asio::serial_port_base::character_size(8),
                boost::asio::serial_port_base::flow_control optFlowControl = boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none),
                boost::asio::serial_port_base::parity optParity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none),
                boost::asio::serial_port_base::stop_bits optStopBits = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));

    virtual void clearBuffers(void);

};

#endif
