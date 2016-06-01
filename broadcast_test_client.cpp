//
//  main.cpp
//  broadcast_send
//
//  Created by 周满满 on 5/28/16.
//  Copyright © 2016 周满满. All rights reserved.
//
// 发送端
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "broadcast.h"

void OnRecieved(OS_UdpSocket& sender, std::string ip, int port, const char* buf, void* userData);

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "port 9888!\n";
    
    ZBroadcast* bc = new ZBroadcastClient(9888);
    bc->regeditRecieve(OnRecieved, NULL);
    bc->start();
    
    std::string msg = "[FileClient]";
    int ret = bc->SendTo(msg.c_str(), msg.length(), 6000);
    if(ret == -1)
    {
        std::cout<<"set socket error..."<<std::endl;
        return -1;
    }
    
    getchar();
    bc->end();
    delete bc;
    
    return 0;
}

void OnRecieved(OS_UdpSocket& sender, std::string ip, int port, const char* buf, void* userData)
{
    if (strcmp(buf, "[FileServer]") == 0)
    {
        printf("recieved message: %s, from ip:%s port:%d\n", buf, ip.c_str(), port);
    }

}
