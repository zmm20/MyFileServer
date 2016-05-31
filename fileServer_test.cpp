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

using namespace std;

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("接收方: port=9000 ...\n");
    
    ZFileServer* fs = new ZUDPFileServer(9000);
    fs->setPath("/Users/zmm/Downloads/");
    
    unsigned long elapse = time(NULL);
    fs->start();
    elapse = time(NULL) - elapse;
    printf("接收完成！elapsed time = %ld\n", elapse);

    return 0;
}