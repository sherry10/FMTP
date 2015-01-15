#ifndef TCPRECV_H_
#define TCPRECV_H_

#include <string>

using namespace std;

class TcpRecv
{
    public:
        TcpRecv(string tcpAddr, unsigned short tcpPort);
        ~TcpRecv();
        ssize_t sendData(void* header, size_t headLen, char* payload,
                         size_t payLen);
        ssize_t recvData(void* header, size_t headLen, char* payload,
                         size_t payLen);
    private:
        int sockfd;
        struct sockaddr_in servAddr;
};

#endif /* TCPRECV_H_ */
