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
// boost_communication.h
//
// University of Colorado, Autonomous Vehicle Systems (AVS) Lab
// Unpublished Copyright (c) 2012-2013 University of Colorado, All Rights Reserved
//

#ifndef __BOOST_COMMUNICATION__
#define __BOOST_COMMUNICATION__

#include <iostream>
#include <cstddef>
#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "definitions.h"

#include "boost/date_time/posix_time/posix_time_types.hpp"

#ifndef MIN
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef CHAR_BUFFER_SIZE
#define CHAR_BUFFER_SIZE        2048
#endif

#define TCPIP_PORT              "50100"
#define MAX_CONNECT_ATTEMPTS    50
#define SERIAL_BAUD_RATE        115200

extern float _floats[MAX_FLOATS];
extern int   _ints[MAX_FLOATS];
extern char  _ip_address[CHAR_BUFFER_SIZE];

typedef enum ComStatus_t {
    COM_INIT,
    COM_SHUTDOWN,
    COM_SEND,
    COM_RECEIVE,
    MAX_COM_STATUS
} ComStatus_t;

int TCPIP_OpenGL(ComStatus_t comStatus, unsigned char *message, size_t sendSize, size_t *inputSize);

/* Templated class must be defined in h file */
class BasicIOObject
{
public:
    BasicIOObject(boost::shared_ptr<boost::asio::io_service> io_service)
        : io_service_(io_service) {
    }

    virtual ~BasicIOObject() {}

    virtual bool isOpen(void) = 0;
    virtual int receiveData(unsigned char *data, size_t *dataSize) = 0;
    virtual int sendData(unsigned char *data, size_t dataSize) = 0;
    virtual int close(void) = 0;

    virtual void check_for_data(int *errorCode) = 0;
    virtual void handle_device(const boost::system::error_code &error, size_t bytes_transferred,
        int *errorCode) = 0;
    virtual void clear_buffers() = 0;
    virtual void handle_clear(const boost::system::error_code &ec,
                              size_t bytes_transferred) = 0;

protected:
    boost::shared_ptr<boost::asio::io_service> io_service_;
};

/* Templated class must be defined in h file */
template<typename SyncWriteStream>
class BasicIOObject_t
    : public BasicIOObject
{
public:
    BasicIOObject_t(boost::shared_ptr<boost::asio::io_service> io_service)
        : BasicIOObject(io_service)
        , inputBufferSize_(0)
        , storedBufferSize_(0)
        , storedBufferWrites_(0) {
        // Initialize buffers as empty
        memset(inputBuffer_, 0, sizeof(inputBuffer_));
        memset(storedBuffer_, 0, sizeof(storedBuffer_));
    }

    virtual ~BasicIOObject_t() {
        close();
    }

    bool isOpen(void) {
        return stream_->is_open();
    }

    int receiveData(unsigned char *data, size_t *dataSize) {
        boost::system::error_code ec;
        inputBufferSize_ = stream_->read_some(boost::asio::buffer(inputBuffer_, CHAR_BUFFER_SIZE), ec);
        if(ec) {
            std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
            return 1;
        }
        for(size_t i = 0; i < inputBufferSize_; i++) {
            data[i] = (unsigned char)inputBuffer_[i];
        }
        *dataSize = inputBufferSize_;

        return 0;
    }

    int sendData(unsigned char *data, size_t dataSize) {
        boost::system::error_code ec;
        boost::asio::write(*stream_, boost::asio::buffer(data, (size_t) dataSize), ec);
        if(ec) {
            std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
            return 1;
        }
        return 0;
    }

    virtual int close(void) {
        if(isOpen()) {
            boost::system::error_code ec;
            stream_->close(ec);
            if(ec) {
                std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
                return 1;
            }
        }
        return 0;
    }

    void check_for_data(int *errorCode) {
        stream_->async_read_some(boost::asio::buffer(inputBuffer_, CHAR_BUFFER_SIZE),
                                 boost::bind(&BasicIOObject::handle_device, this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred,
                                             errorCode));
    }

    void handle_device(const boost::system::error_code &ec, size_t bytes_transferred, int *errorCode) {
        if(!ec) {
            inputBufferSize_ = bytes_transferred;
            size_t maxStoredBufferWrites = 1;
            size_t newBytes = MIN(inputBufferSize_ + storedBufferSize_, sizeof(storedBuffer_)) - storedBufferSize_;
            for(size_t i = 0; i < newBytes; i++) {
                storedBuffer_[storedBufferSize_++] = inputBuffer_[i];
            }

            if(storedBufferSize_ != EXPECTED_MSG_SIZE) {
                if(storedBufferWrites_ >= maxStoredBufferWrites) {
                    printf("Warning: Received %lu bytes instead of %d, clearing.\n", storedBufferSize_, EXPECTED_MSG_SIZE);
                    storedBufferSize_ = 0;
                    storedBufferWrites_ = 0;
                    memset(storedBuffer_, 0, sizeof(storedBuffer_));
                } else {
                    printf("Warning: Received %lu bytes instead of %d, waiting (%lu) for more.\n",
                        storedBufferSize_, EXPECTED_MSG_SIZE, maxStoredBufferWrites - storedBufferWrites_);
                    storedBufferWrites_++;
                }
            } else {
                if(storedBufferWrites_ > 0) {
                    printf("Success: Received proper bytes after %lu reads\n", storedBufferWrites_);
                }

                sscanf(storedBuffer_, "%d,"
                                    "%f,"
                                    "%f,"
                                    "%f,%f,%f,"
                                    "%f,%f,%f,"
                                    "%f,%f,%f,"
                                    "%f,%f,%f,%f,"
                                    "%f,%f,%f,%f,"
                                    "%f,%f,%f,%f,"
                                    "%f,%f,%f,"
                                    "%d,%d,%d,%d,"
                                    "%d,%d,%d,%d,"
                                    "%d,%d,%d,"
                                    "%d,"
                                    "%d,"
                                    "%d,"
                                    "%d,"
                                    "%d,"
                                    "%d,%d,%d,%d,%d,%d,%d,%d,"
                                    "%f,%f,%f,"
                                    "%f,"
                                    "%d,"
                                    "%f,"
                                    "%f,"
                                    "%f,%f,%f,"
                                    "%d",
                                     &HASSIMENDED,
                                     &TIME,
                                     &GAMMA,
                                     &SUN_X, &SUN_Y, &SUN_Z,
                                     &POSITION_X, &POSITION_Y, &POSITION_Z,
                                     &VELOCITY_X, &VELOCITY_Y, &VELOCITY_Z,
                                     &ROTATION_PHI, &ROTATION_EX, &ROTATION_EY, &ROTATION_EZ,
                                     &RW1_TORQUE, &RW2_TORQUE, &RW3_TORQUE, &RW4_TORQUE,
                                     &RW1_SPEED, &RW2_SPEED, &RW3_SPEED, &RW4_SPEED,
                                     &MTB1_CURRENT, &MTB2_CURRENT, &MTB3_CURRENT,
                                     &RW1_STATE, &RW2_STATE, &RW3_STATE, &RW4_STATE,
                                     &RW1_SPEEDSTATE, &RW2_SPEEDSTATE, &RW3_SPEEDSTATE, &RW4_SPEEDSTATE,
                                     &MTB1_STATE, &MTB2_STATE, &MTB3_STATE,
                                     &ST_STATE,
                                     &IRU_STATE,
                                     &TAM_STATE,
                                     &POD_STATE,
                                     &DEGAUSSING_FLAG,
                                     &CSS1_STATE, &CSS2_STATE, &CSS3_STATE, &CSS4_STATE,
                                     &CSS5_STATE, &CSS6_STATE, &CSS7_STATE, &CSS8_STATE,
                                     &MAG_X, &MAG_Y, &MAG_Z,
                                     &SUN_ANGLE,
                                     &THRUST_STATE,
                                     &NADIR_ANGLE,
                                     &THRUST_ANGLE,
                                     &SUN_BX, &SUN_BY, &SUN_BZ,
                                     &ADCS_STATE);
                
                /* Clear buffer now that it has been read */
                storedBufferSize_ = 0;
                storedBufferWrites_ = 0;
                memset(storedBuffer_, 0, sizeof(storedBuffer_));
            
            }
            *errorCode = 0;
            check_for_data(errorCode);

        } else {
            std::cout << "Error in " << __FUNCTION__ 
                      << " (" << ec.category().name() << ":" << ec.value() << ") " << ec.message() << std::endl;
            *errorCode = 1;
        }
    }

    void clear_buffers(void) {
        stream_->async_read_some(boost::asio::buffer(inputBuffer_, CHAR_BUFFER_SIZE),
                                 boost::bind(&BasicIOObject::handle_clear, this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    }

    void handle_clear(const boost::system::error_code &ec,
                      size_t bytes_transferred) {
        inputBufferSize_ = bytes_transferred;
        if(ec) {
            std::cout << "Error in " << __FUNCTION__ << " (" << ec.value() << ") " << ec.message() << std::endl;
        }
    }

protected:
    boost::shared_ptr<SyncWriteStream> stream_;
    char inputBuffer_[CHAR_BUFFER_SIZE];
    size_t inputBufferSize_;
    char storedBuffer_[CHAR_BUFFER_SIZE];
    size_t storedBufferSize_;
    size_t storedBufferWrites_;
};

// Server only accepts one connection
class TcpServer
    : public BasicIOObject_t<boost::asio::ip::tcp::socket>
{
public:
    TcpServer(boost::shared_ptr<boost::asio::io_service> io_service);

    int startListening(std::string ipAddress, std::string portNum);
    int acceptConnection(void);

    virtual int close(void);

private:
    boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    boost::asio::ip::tcp::endpoint endpoint_;
};

class TcpClient
    : public BasicIOObject_t<boost::asio::ip::tcp::socket>
{
public:
    TcpClient(boost::shared_ptr<boost::asio::io_service> io_service);

    int connect(std::string ipAddress, std::string portNum);

private:

};

class SerialConnection
    : public BasicIOObject_t<boost::asio::serial_port>
{
public:
    SerialConnection(boost::shared_ptr<boost::asio::io_service> io_service);

    int connect(std::string deviceName,
                unsigned int baudRate = SERIAL_BAUD_RATE,
                boost::asio::serial_port_base::character_size optCharSize = boost::asio::serial_port_base::character_size(8),
                boost::asio::serial_port_base::flow_control optFlowControl = boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none),
                boost::asio::serial_port_base::parity optParity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none),
                boost::asio::serial_port_base::stop_bits optStopBits = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));

private:

};

#endif
