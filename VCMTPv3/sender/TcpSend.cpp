/**
 * Copyright (C) 2014 University of Virginia. All rights reserved.
 *
 * @file      TcpSend.cpp
 * @author    Shawn Chen <sc7cq@virginia.edu>
 * @version   1.0
 * @date      Nov 17, 2014
 *
 * @section   LICENSE
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or（at your option）
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details at http://www.gnu.org/copyleft/gpl.html
 *
 * @brief     Implement the interfaces and structures of sender side TCP layer
 *            abstracted funtions.
 *
 * The TcpSend class includes a set of transmission functions, which are
 * basically the encapsulation of tcp system calls themselves. This abstracted
 * new layer acts as the sender side transmission library.
 */



#include <errno.h>
#include <iostream>
#include <sys/socket.h>
#include <mutex>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <system_error>
#include "TcpSend.h"
#include "vcmtpBase.h"
#include <sys/uio.h>
#include <stdexcept>

#ifndef NULL
    #define NULL 0
#endif
#define MAX_CONNECTION 50

using namespace std;

/**
 * Contructor for TcpSend class, taking tcp address and tcp port to establish a
 * tcp connection. When the connection is established, keep listen on it with a
 * maximum of MAX_CONNECTION allowed to connect.
 *
 * @param[in] tcpAddr     Specification of the interface on which the TCP
 *                        server will listen as a dotted-decimal IPv4 address.
 * @param[in] tcpPort     tcp port number (in host order) specified by sending
 *                        application. (or 0, meaning system will use random
 *                        available port)
 * @throw  std::invalid_argument if `tcpAddr` is invalid.
 * @throw  std::runtime_error    if socket creation fails.
 * @throw  std::runtime_error    if socket bind() operation fails.
 */
TcpSend::TcpSend(string tcpAddr, unsigned short tcpPort)
    : sockListMutex()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        throw std::runtime_error("TcpSend::TcpSend() error creating socket");
    (void) memset((char *) &servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    in_addr_t inAddr = inet_addr(tcpAddr.c_str());
    if ((in_addr_t)(-1) == inAddr)
        throw std::invalid_argument(std::string("Invalid interface: ") +
                tcpAddr);
    servAddr.sin_addr.s_addr = inAddr;
    /** If tcpPort = 0, OS will automatically choose an available port number. */
    servAddr.sin_port = htons(tcpPort);
#if 0
    cerr << std::string("TcpSend::TcpSend() Binding TCP socket to ").
            append(tcpAddr).append(":").append(std::to_string(tcpPort)).
            append("\n");
#endif
    if(::bind(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        throw std::system_error(errno, std::system_category(),
                "TcpSend::TcpSend(): Couldn't bind \"" + tcpAddr + ":" +
                std::to_string(tcpPort) + "\"");
    /** listen() returns right away, it's non-blocking */
    listen(sockfd, MAX_CONNECTION);
}


/**
 * Destructor for TcpSend class. Release all the allocated resources, including
 * mutex, socket lists and etc.
 *
 * @param[in] none
 */
TcpSend::~TcpSend()
{
    for (list<int>::iterator it = connSockList.begin();
         it != connSockList.end(); ++it)
    {
        connSockList.erase(it);
    }
    close(sockfd);
}


/**
 * Accept incoming tcp connection requests and push them into the socket list.
 * Then return the current socket file descriptor for further use. The socket
 * list is a globally shared resource, thus it needs to be protected by a lock.
 *
 * @param[in] none
 * @return    newsockfd         file descriptor of the newly connected socket.
 * @throw  runtime_error    if accept() system call fails.
 */
int TcpSend::acceptConn()
{
    struct sockaddr_in addr;
    unsigned           addrLen = sizeof(addr);
    if (getsockname(sockfd, (struct sockaddr*)&addr, &addrLen))
        throw std::system_error(errno, std::system_category(),
                std::string("Couldn't get address of socket ") +
                std::to_string(sockfd));
#if 0
    cerr << std::string("TcpSend::acceptConn(): Accept()ing on socket ").
            append(std::to_string(sockfd)).append(" (").
            append(inet_ntoa(addr.sin_addr)).append(":").
            append(std::to_string(ntohs(addr.sin_port))).append(")\n");
#endif
    int newsockfd = accept(sockfd, NULL, NULL);
    if(newsockfd < 0)
        throw std::runtime_error("TcpSend::acceptConn() error reading from socket");

#if 0
    cerr << std::string("TcpSend::acceptConn(): Accepted new socket ").
            append(std::to_string(newsockfd)).append("\n");
#endif
    {
        std::unique_lock<std::mutex> lock(sockListMutex);
        connSockList.push_back(newsockfd);
    }

    return newsockfd;
}


/**
 * Accept incoming tcp connection requests and push them into the socket list.
 * Then return the current socket file descriptor for further use.
 *
 * @param[in] none
 * @return    connSockList          connected socket list (a collection of
 */
const list<int>& TcpSend::getConnSockList()
{
    return connSockList;
}


/**
 * Read a given amount of bytes from the socket.
 *
 * @param[in] retxsockfd    retransmission socket file descriptor.
 * @param[in] *pktBuf       pointer to the buffer to store the received bytes.
 * @param[in] bufSize       size of that buffer
 * @return    int           return the return value of read() system call.
 */
int TcpSend::readSock(int retxsockfd, char* pktBuf, int bufSize)
{
    return read(retxsockfd, pktBuf, bufSize);
}


/**
 * Read an amount of bytes from the socket while the number of bytes equals the
 * VCMTP header size. Parse the buffer which stores the packet header and fill
 * each field of VcmtpHeader structure with corresponding information. If the
 * read() system call fails, return immediately. Otherwise, return when this
 * function finishes.
 *
 * @param[in] retxsockfd    retransmission socket file descriptor.
 * @param[in] *recvheader   pointer of a VcmtpHeader structure, whose fields
 *                          are to hold the parsed out information.
 * @return    retval        return the status value returned by read()
 */
int TcpSend::parseHeader(int retxsockfd, VcmtpHeader* recvheader)
{
    char recvbuf[VCMTP_HEADER_LEN];
    int retval = read(retxsockfd, recvbuf, sizeof(recvbuf));
    if(retval < 0)
        return retval;

    memcpy(&recvheader->prodindex,  recvbuf,    4);
    memcpy(&recvheader->seqnum,     recvbuf+4,  4);
    memcpy(&recvheader->payloadlen, recvbuf+8,  2);
    memcpy(&recvheader->flags,      recvbuf+10, 2);
    recvheader->prodindex  = ntohl(recvheader->prodindex);
    recvheader->seqnum     = ntohl(recvheader->seqnum);
    recvheader->payloadlen = ntohs(recvheader->payloadlen);
    recvheader->flags      = ntohs(recvheader->flags);
    return retval;
}


/**
 * Send a VCMTP packet through the given retransmission connection identified
 * by the passed-in socket file descriptor. This function calls the writev()
 * system call to implement a zero-copy gathering write operation where the
 * packet header and payload are stored in different places. After the writev()
 * is finished, return the status value of the writev() system call.
 *
 * @param[in] retxsockfd    retransmission socket file descriptor.
 * @param[in] *sendheader   pointer of a VcmtpHeader structure, whose fields
 *                          are to hold the ready-to-send information.
 * @param[in] *payload      pointer to the ready-to-send memory buffer which
 *                          holds the packet payload.
 * @param[in] paylen        size to be sent (size of the payload)
 * @return    retval        return the status value returned by writev()
 */
int TcpSend::send(int retxsockfd, VcmtpHeader* sendheader, char* payload,
                  size_t paylen)
{
    struct iovec iov[2];

    iov[0].iov_base = sendheader;
    iov[0].iov_len  = sizeof(VcmtpHeader);
    iov[1].iov_base = payload;
    iov[1].iov_len  = paylen;

    int retval = writev(retxsockfd, iov, 2);
    return retval;
}


/**
 * Return the local port number.
 *
 * @return                The local port number in host byte-order.
 * @throw std::system_error  The port number cannot be obtained.
 */
unsigned short TcpSend::getPortNum()
{
    struct sockaddr_in tmpAddr;
    socklen_t          tmpAddrLen = sizeof(tmpAddr);

    if (getsockname(sockfd, (struct sockaddr*)&tmpAddr, &tmpAddrLen) < 0)
        throw std::runtime_error("TcpSend::getPortNum() error getting port \
                                 number");

    return ntohs(tmpAddr.sin_port);
}
