/*
*UdpSocket.cpp
 *
 *  Created on: Oct 16, 2014
 *      Author: fatmaal-ali
 */

#include "UdpSocket.h"

/**
 * Set the IP address and port of the receiver and connect to the upd socket.
 *
 * @param[in] recvAddr     IP address of the receiver.
 * @param[in] port         Port number of the receiver.
 */
UdpSocket::UdpSocket(const char* recvAddr, unsigned short port) {
    /** create a UDP datagram socket. */
    if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cout<<"UdpSocket::UdpSocket error";
    }
    /** clear struct recv_addr. */
    bzero(&recv_addr, sizeof(recv_addr));
    /** set connection type to IPv4 */
    recv_addr.sin_family = AF_INET;
    /** set the address to the receiver address passed to the constructor */
    recv_addr.sin_addr.s_addr =inet_addr(recvAddr);
    /** set the port number to the port number passed to the constructor */
    recv_addr.sin_port = htons(port);
    connect(sock_fd, (struct sockaddr *) &recv_addr, sizeof(recv_addr));
}

UdpSocket::~UdpSocket() {
    // TODO Auto-generated destructor stub
}

ssize_t UdpSocket::SendTo(const void* buff, size_t len)
{
    return send(sock_fd, buff, len, 0);
}

ssize_t UdpSocket::SendData(char* header, const size_t headerLen, char* data, const size_t dataLen)
{
    int ret;
    /** vector including the two memory locations */
    struct iovec iov[2];
    iov[0].iov_base = header;
    iov[0].iov_len  = headerLen;
    iov[1].iov_base = data;
    iov[1].iov_len  = dataLen;


    ret = writev(sock_fd, iov, 2);
    return ret;
}
