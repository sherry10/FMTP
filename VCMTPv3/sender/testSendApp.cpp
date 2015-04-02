/**
 * Copyright (C) 2014 University of Virginia. All rights reserved.
 *
 * @file      testSendApp.cpp
 * @author    Shawn Chen <sc7cq@virginia.edu>
 * @version   1.0
 * @date      Feb 15, 2015
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
 * @brief     A testing application to use the sender side protocol.
 *
 * Since the LDM could be too heavy to use for testing purposes only. This
 * testSendApp is written as a replacement. It can create an instance of the
 * vcmtpSendv3 instance and mock the necessary components to get it functioning.
 */


#include "vcmtpSendv3.h"

#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <string>


/**
 * Since the LDM could be too heavy to use for testing purposes only. This main
 * function is a light weight replacement of the LDM sending application. It
 * sets up the whole environment and call sendProduct() to send data. All the
 * arguments are passed in through command line.
 *
 * @param[in] tcpAddr      IP address of the sender.
 * @param[in] tcpPort      Port number of the sender.
 * @param[in] mcastAddr    multicast address of the group.
 * @param[in] mcastPort    Port number of the multicast group.
 * @param[in] filename     file to be sent as data.
 */
int main(int argc, char const* argv[])
{
    if (argc < 4) {
        std::cerr << "ERROR: Insufficient arguments." << std::endl;
        return 1;
    }

    std::string tcpAddr(argv[1]);
    const unsigned short tcpPort = (unsigned short)atoi(argv[2]);
    std::string mcastAddr(argv[3]);
    const unsigned short mcastPort = (unsigned short)atoi(argv[4]);
    std::string filename(argv[5]);

    char tmp[] = "test metadata";
    char* metadata = tmp;
    unsigned int metaSize = sizeof(tmp);

    vcmtpSendv3* sender =
        new vcmtpSendv3(tcpAddr.c_str(), tcpPort, mcastAddr.c_str(), mcastPort,
                        0, 0);

    /** use the filename to get filesize */
    struct stat filestatus;
    stat(filename.c_str(), &filestatus);
    size_t datasize = filestatus.st_size;

    int fd = open(filename.c_str(), O_RDONLY);
    if(fd>0)
    {
        void* data = (char*) mmap(0, datasize, PROT_READ, MAP_FILE | MAP_SHARED,
                                  fd, 0);
        if (data == MAP_FAILED)
            std::cerr << "file map failed" << std::endl;

        sender->Start();
        sleep(2);
        for(int i=0; i<100; ++i) {
            sender->sendProduct(data, datasize, metadata, metaSize);
            sleep(1);
        }
        while(1);

        munmap(data, datasize);
        close(fd);
    }
    else
        std::cerr << "test::main()::open(): error" << std::endl;

    delete sender;
    return 0;
}
