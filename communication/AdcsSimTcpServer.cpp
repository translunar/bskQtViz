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
// AdcsSimTcpServer.cpp
//
// University of Colorado, Autonomous Vehicle Systems (AVS) Lab
// Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
//

#include "AdcsSimTcpServer.hpp"


#include "SpacecraftSimDefinitions.h"
//#include "rwRead.hpp"
//#include "CDH.hpp"
//#include "stRead.hpp"
//#include "TAM.hpp"
extern "C" {
#include "spacecraftDefinitions.h"
}

std::string serverDevice2str(ServerDevice_t device)
{
    switch(device) {
        case DEVICE_RW1:
            return "RW1";
        case DEVICE_RW2:
            return "RW2";
        case DEVICE_RW3:
            return "RW3";
        case DEVICE_RW4:
            return "RW4";
        case DEVICE_ST:
            return "ST";
        case DEVICE_IRU:
            return "IRU";
        case DEVICE_TAM:
            return "TAM";
        case DEVICE_POD:
            return "POD";
        case DEVICE_CDH:
            return "CDH";
        case MAX_SERVER_DEVICE:
        default:
            return "MAX_SERVER_DEVICE";
    }
}

AdcsSimTcpServer::AdcsSimTcpServer(boost::asio::io_service *ioService)
    : TcpServer(ioService)
    , m_storedBufferWrites(0)
{
}

AdcsSimTcpServer::~AdcsSimTcpServer()
{
}

void AdcsSimTcpServer::checkForAsyncData(ServerDevice_t device, SpacecraftSim *scSim)
{
    m_stream->async_read_some(boost::asio::buffer(m_inboundBuffer),
                              boost::bind(&AdcsSimTcpServer::handleCheckForAsyncData, this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred,
                                          device, scSim));
}

void AdcsSimTcpServer::handleCheckForAsyncData(const boost::system::error_code &ec,
        size_t bytes_transferred,
        ServerDevice_t device,
        SpacecraftSim *scSim)
{
    if(!ec) {
        size_t maxStoredBufferWrites = 1;
        size_t newBytes = (size_t)fmin(m_inboundBuffer.size() + m_storedBufferSize, sizeof(m_storedBuffer)) - m_storedBufferSize;
        for(size_t i = 0; i < newBytes; i++) {
            m_storedBuffer[m_storedBufferSize++] = m_inboundBuffer[i];
        }

//        switch(device) {
//            case DEVICE_RW1:
//            case DEVICE_RW2:
//            case DEVICE_RW3:
//            case DEVICE_RW4: {
//                int i = device - DEVICE_RW1;
//                if(m_storedBufferSize != RW_CMD_CODE_SIZE && m_storedBufferSize != RW_CMD_CODE_SIZE + RW_CMD_SIZE) {
//                    if(m_storedBufferWrites >= maxStoredBufferWrites) {
//                        printf("Warning: RW%d has received %lu bytes instead of %d/%d, clearing.\n",
//                               i, m_storedBufferSize, RW_CMD_CODE_SIZE, RW_CMD_CODE_SIZE + RW_CMD_SIZE);
//                        m_storedBufferSize = 0;
//                        m_storedBufferWrites = 0;
//                        memset(m_storedBuffer, 0, sizeof(m_storedBuffer));
//                    } else {
//                        m_storedBufferWrites++;
//                    }
//                } else {
//                    scSim->rw[i].inputLength = MIN(m_storedBufferSize, sizeof(scSim->rw[i].input));
//                    memcpy(scSim->rw[i].input, m_storedBuffer, scSim->rw[i].inputLength);
//
//                    rwReply(i, scSim);
//
//                    std::string messageStr((char *)scSim->rw[i].output, scSim->rw[i].outputLength);
//                    if(sendData(messageStr) != 0) {
//                        std::cout << "called by " << __FUNCTION__ << std::endl;
//                        return;
//                    }
//                    if(scSim->displayDeviceReturnString) {
//                        printf("RW%1d: ", i + 1);
//                        printHexString(scSim->rw[i].output, scSim->rw[i].outputLength);
//                        printf("\n");
//                    }
//                    m_storedBufferSize = 0;
//                    m_storedBufferWrites = 0;
//                    memset(m_storedBuffer, 0, sizeof(m_storedBuffer));
//                }
//                break;
//            }
//
//            case DEVICE_CDH: {
//                /* CDH command is assumed to include:
//                * 4 character sync mark (1A CF FC 1D)
//                * adcsState
//                * st state
//                * iru state
//                * tam state
//                * TR commands (4*3)
//                * TR flags (3)
//                * hasSimEnded flag
//                * rw states (4)
//                * tr states (3)
//                * pod state
//                * css states (8)
//                * spare (4)
//                * 4 character end mark (AA 55 A5 5A) */
//                /* TODO (WARNING): Reality on demand functionality hacked to make flatsat work */
//                size_t trCommandLength = sizeof(scSim->tr[0].input);
//                size_t expectedLength = 4 // sync mark
//                                        + 4 // adcs, st, iru, tam states
//                                        + NUM_TR * trCommandLength // tr commands
//                                        + NUM_TR // tr flags
//                                        + 1 // hassimended flag
//                                        + NUM_RW // rw states
//                                        + NUM_TR // tr states
//                                        + 1 // spare
//                                        + NUM_CSS // css states
//                                        + 4 // spares
//                                        + 4; // sync mark
//
//                /* Increase max buffer writes because it will take 3 sends */
//                maxStoredBufferWrites = 4;
//
//                if(m_storedBufferSize != expectedLength) {
//                    if(m_storedBufferWrites >= maxStoredBufferWrites) {
//                        if(m_storedBufferWrites >= maxStoredBufferWrites) {
//                            printf("Warning: CDH has received %lu bytes instead of %lu, clearing.\n",
//                                   m_storedBufferSize, expectedLength);
//                            m_storedBufferSize = 0;
//                            m_storedBufferWrites = 0;
//                            memset(m_storedBuffer, 0, sizeof(m_storedBuffer));
//                        } else {
//                            m_storedBufferWrites++;
//                        }
//                    } else {
//                        if(scSim->displayDeviceReturnString) {
//                            printf("CDH: ");
//                            printHexString((unsigned char *)m_storedBuffer, m_storedBufferSize);
//                            printf("\n");
//                        }
//
//                        size_t iter = 0;
//                        /* read and check the sync mark */
//                        unsigned char syncMark[4];
//                        memcpyEndian(syncMark, &m_storedBuffer[iter], sizeof(syncMark));
//                        iter += 4;
//                        /* TODO: Do some error checking here */
//
//                        scSim->adcsState = (ADCSState_t)m_storedBuffer[iter++];
//                        scSim->st.state = (ComponentState_t)m_storedBuffer[iter++];
//                        scSim->iru.state = (ComponentState_t)m_storedBuffer[iter++];
//                        scSim->tam.state = (ComponentState_t)m_storedBuffer[iter++];
//
//                        /* set the TR current commands */
//                        for(int i = 0; i < NUM_TR; i++) {
//                            scSim->tr[i].inputLength = trCommandLength;
//                            memcpy(scSim->tr[i].input, &m_storedBuffer[iter], trCommandLength);
//                            iter += trCommandLength;
//                            /* Set command into scSim.tr.u and scSim.tr.current */
//                            scSim->mtbReply(i);
//                        }
//
//                        /* set the TR status bytes */
//                        for(int i = 0; i < NUM_TR; i++) {
//                            scSim->tr[i].flags = (unsigned char)m_storedBuffer[iter++];
//                        }
//
//                        /* set the end of simulation flag */
//                        scSim->hasSimEnded = (unsigned char)m_storedBuffer[iter++];
//
//                        for(int i = 0; i < NUM_RW; i++) {
//                            scSim->rw[i].state = (ComponentState_t)m_storedBuffer[iter++];
//                        }
//                        for(int i = 0; i < NUM_TR; i++) {
//                            scSim->tr[i].state = (ComponentState_t)m_storedBuffer[iter++];
//                        }
//
//                        scSim->pod.state = (ComponentState_t)m_storedBuffer[iter++];
//
//                        for(int i = 0; i < NUM_CSS; i++) {
//                            scSim->css[i].state = (ComponentState_t)m_storedBuffer[iter++];
//                        }
//
//                        /* Skip over spare bytes */
//                        iter += 4;
//
//                        /* read and check the end sync mark */
//                        memcpyEndian(syncMark, &m_storedBuffer[iter], sizeof(syncMark));
//                        iter += 4;
//                        /* TODO: Do some error checking here */
//
//                        m_storedBufferSize = 0;
//                        m_storedBufferWrites = 0;
//                        memset(m_storedBuffer, 0, sizeof(m_storedBuffer));
//                    }
//                    break;
//                }
//            }
//
//            case DEVICE_ST: {
//                if(m_storedBufferSize != sizeof(scSim->st.input)) {
//                    if(m_storedBufferWrites >= maxStoredBufferWrites) {
//                        printf("Warning: ST has received %lu bytes instead of %lu, clearing.\n",
//                               m_storedBufferSize, sizeof(scSim->st.input));
//                        m_storedBufferSize = 0;
//                        m_storedBufferWrites = 0;
//                        memset(m_storedBuffer, 0, sizeof(m_storedBuffer));
//                    } else {
//                        m_storedBufferWrites++;
//                    }
//                } else {
//                    scSim->st.inputLength = MIN(m_storedBufferSize, sizeof(scSim->st.input));
//                    memcpy(scSim->st.input, m_storedBuffer, scSim->st.inputLength);
//
//                    stReply(scSim);
//
//                    std::string messageStr((char *)scSim->st.output, scSim->st.outputLength);
//                    if(sendData(messageStr) != 0) {
//                        std::cout << "called by " << __FUNCTION__ << std::endl;
//                        return;
//                    }
//                    if(scSim->displayDeviceReturnString) {
//                        printf("ST: ");
//                        printHexString(scSim->st.output, scSim->st.outputLength);
//                        printf("\n");
//                    }
//
//                    m_storedBufferSize = 0;
//                    m_storedBufferWrites = 0;
//                    memset(m_storedBuffer, 0, sizeof(m_storedBuffer));
//                }
//                break;
//            }
//
//            case DEVICE_TAM: {
//                if(m_storedBufferSize != sizeof(scSim->tam.input)) {
//                    if(m_storedBufferWrites >= maxStoredBufferWrites) {
//                        printf("Warning: TAM has received %lu bytes instead of %lu, clearing.\n",
//                               m_storedBufferSize, sizeof(scSim->tam.input));
//                        m_storedBufferSize = 0;
//                        m_storedBufferWrites = 0;
//                        memset(m_storedBuffer, 0, sizeof(m_storedBuffer));
//                    } else {
//                        m_storedBufferWrites++;
//                    }
//                } else {
//                    scSim->tam.inputLength = MIN(m_storedBufferSize, sizeof(scSim->tam.input));
//                    memcpy(scSim->tam.input, m_storedBuffer, scSim->tam.inputLength);
//
//                    scSim->tam.tamReply(scSim);
//
//                    std::string messageStr((char *)scSim->tam.output, scSim->tam.outputLength);
//                    if(sendData(messageStr) != 0) {
//                        std::cout << "called by " << __FUNCTION__ << std::endl;
//                        return;
//                    }
//                    if(scSim->displayDeviceReturnString) {
//                        printf("TAM: ");
//                        printHexString(scSim->tam.output, (size_t)scSim->tam.outputLength);
//                        printf("\n");
//                    }
//
//                    m_storedBufferSize = 0;
//                    m_storedBufferWrites = 0;
//                    memset(m_storedBuffer, 0, sizeof(m_storedBuffer));
//                }
//                break;
//            }
//
//            default:
//                std::cout << "Error: unmodeled device type " << serverDevice2str(device) << " received data in handleCheckForAsyncData().\n";
//                break;
//        }
        checkForAsyncData(device, scSim);

    } else {
        std::cout << "Error for " << serverDevice2str(device) << " in " << __FUNCTION__
                  << " (" << ec.category().name() << ":" << ec.value() << ") " << ec.message() << std::endl;
    }
}
