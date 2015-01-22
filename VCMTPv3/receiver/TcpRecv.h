#ifndef TCPRECV_H_
#define TCPRECV_H_

#include <mutex>
#include <string>

class TcpRecv
{
    public:
        TcpRecv(std::string tcpAddr, unsigned short tcpPort);
        ~TcpRecv();
        /**
         * Sends a header and a payload on the TCP connection. Blocks until the packet
         * is sent or a severe error occurs. Re-establishes the TCP connection if
         * necessary.
         *
         * @param[in] header   Header.
         * @param[in] headLen  Length of the header in bytes.
         * @param[in] payload  Payload.
         * @param[in] payLen   Length of the payload in bytes.
         * @retval    -1       O/S failure.
         * @return             Number of bytes sent.
         */
        ssize_t sendData(void* header, size_t headLen, char* payload,
                         size_t payLen);
        /**
         * Receives a header and a payload on the TCP connection. Blocks until the
         * packet is received or a severe error occurs. Re-establishes the TCP
         * connection if necessary.
         *
         * @param[in] header   Header.
         * @param[in] headLen  Length of the header in bytes.
         * @param[in] payload  Payload.
         * @param[in] payLen   Length of the payload in bytes.
         * @retval    -1       O/S failure.
         * @return             Number of bytes received.
         */
        ssize_t recvData(void* header, size_t headLen, char* payload,
                         size_t payLen);
    private:
        int                     sockfd;
        struct sockaddr_in      servAddr;
        std::mutex              mutex;

        /**
         * Initializes the TCP connection. Blocks until the connection is established
         * or a severe error occurs.
         *
         * @throws std::system_error  if a system error occurs.
         */
        void initSocket();
        /**
         * Ensures that the TCP connection is established. Blocks until the connection
         * is established or a severe error occurs. Does nothing if the connection is
         * OK. This method is thread-safe.
         *
         * @throws std::system_error  if a system error occurs.
         */
        void reconnect();
        /**
         * Returns the socket corresponding to the TCP connection. This method is
         * thread-safe.
         *
         * @return  The corresponding socket.
         */
        int  getSocket();
};

#endif /* TCPRECV_H_ */
