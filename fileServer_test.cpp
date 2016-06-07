//
//  main.cpp
//  TCP_Server
//
//  Created by 周满满 on 6/3/16.
//  Copyright © 2016 周满满. All rights reserved.
//

#include <iostream>
#include "fileServer.h"
#include "broadcast.h"
#define LOCAL_IP "127.0.0.1"
#define ANY_IP "0.0.0.0"

void OnRecieved(OS_UdpSocket& sender, std::string ip, int port, const char* buf, void* userData);
int main(int argc, const char * argv[]) {
    // insert code here...
    printf("开启udp广播端口6000\n");
    ZBroadcast* bs = new ZBroadcastServer("0.0.0.0", 6000);
    bs->regeditRecieve(OnRecieved, NULL);
    bs->start();
    
    std::string ip = LOCAL_IP;
    bool isUdp = false;
    printf("接收方: ip=%s, port=9000, protocal=%s...\n", ip.c_str(), isUdp? "udp" : "tcp/ip");
    ZFileServer* fs;
    if (isUdp)
        fs = new ZUDPFileServer(ip.c_str(), 9000);
    else
        fs = new ZTCPFileServer(ip.c_str(), 9000);
    fs->setPath("/Users/zmm/Downloads/");
    
    unsigned long elapse = time(NULL);
    fs->start();
    elapse = time(NULL) - elapse;
    printf("接收完成！elapsed time = %ld\n", elapse);
    return 0;
}


void OnRecieved(OS_UdpSocket& sender, std::string ip, int port, const char* buf, void* userData)
{
    if (strcmp(buf, "[FileClient]") == 0)
    {
        printf("recieved message: %s, from ip:%s port:%d\n", buf, ip.c_str(), port);
        
        std::string msg = "[FileServer]";
        sender.SendTo(msg.c_str(), msg.length(), OS_SockAddr(ip.c_str(), port));
    }
}

