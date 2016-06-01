//
//  main.cpp
//  UDP_HostB
//
//  Created by 周满满 on 5/19/16.
//  Copyright © 2016 周满满. All rights reserved.
//

#include <iostream>
#include "filePackage.h"
#include <time.h>
#include "fileServer.h"
#include "broadcast.h"

using namespace std;

void OnRecieved(OS_UdpSocket& sender, std::string ip, int port, const char* buf, void* userData);
int main(int argc, const char * argv[]) {
    ZBroadcast* bs = new ZBroadcastServer(6000);
    bs->regeditRecieve(OnRecieved, NULL);
    bs->start();
    
    printf("接收方: port=9000 ...\n");
    
    ZFileServer* fs = new ZUDPFileServer(9000);
    fs->setPath("/Users/zmm/Downloads/");
    
    unsigned long elapse = time(NULL);
    fs->start();
    elapse = time(NULL) - elapse;
    printf("接收完成！elapsed time = %ld\n", elapse);
    
    delete fs;
    bs->end();
    delete bs;

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
