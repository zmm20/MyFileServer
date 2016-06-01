//
//  main.cpp
//  broadcast
//
//  Created by 周满满 on 5/28/16.
//  Copyright © 2016 周满满. All rights reserved.
//
// 接收端 端如下：
#include <iostream>
#include <stdio.h>
#include "osapi/socket.h"
#include "broadcast.h"
#include <string>

void OnRecieved(OS_UdpSocket& sender, std::string ip, int port, const char* buf);
int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "open broadcast port 6000!"<<std::endl;
    
    ZBroadcast* bs = new ZBroadcastServer(6000);
    bs->regeditRecieve(OnRecieved);
    bs->start();
    
    getchar();
    bs->end();
    delete bs;
    
    return 0;
}

void OnRecieved(OS_UdpSocket& sender, std::string ip, int port, const char* buf)
{
    if (strcmp(buf, "[FileClient]") == 0)
    {
        printf("recieved message: %s, from ip:%s port:%d\n", buf, ip.c_str(), port);
        
        std::string msg = "[FileServer]";
        sender.SendTo(msg.c_str(), msg.length(), OS_SockAddr(ip.c_str(), port));
    }
}
