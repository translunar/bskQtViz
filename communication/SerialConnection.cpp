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
// SerialConnection.cpp
//
// University of Colorado, Autonomous Vehicle Systems (AVS) Lab
// Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
//

#include "SerialConnection.hpp"

SerialConnection::SerialConnection(boost::asio::io_service *ioService)
{
    m_stream.reset(new boost::asio::serial_port(*ioService));
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
    m_stream->open(deviceName, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    m_stream->set_option(boost::asio::serial_port_base::baud_rate(baudRate), ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    m_stream->set_option(optCharSize, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    m_stream->set_option(optFlowControl, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    m_stream->set_option(optParity, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    m_stream->set_option(optStopBits, ec);
    if(ec) {
        std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        return 1;
    }

    return 0;
}

void SerialConnection::clearBuffers(void)
{
    m_stream->async_read_some(boost::asio::buffer(m_inboundBuffer),
                              boost::bind(&BasicIoObject_t::handleClearBuffers, this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred));
}
