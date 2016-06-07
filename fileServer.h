//
//  fileServer.h
//  UDP_HostB
//
//  Created by 周满满 on 5/25/16.
//  Copyright © 2016 周满满. All rights reserved.
//

#ifndef fileServer_h
#define fileServer_h
#include <string>
#include <map>
#include <vector>
#include "osapi/socket.h"
#include "filePackage.h"
//ZFileServer 基本用法设计
//ZFileClient* fc = new ZUDPFileServer(self_ip, self_port); // 如果是TCP，则new ZTCPFileClient(self_ip, self_port)
//fc->setPath(...); // 设置路径
//fc->start();

class ZFileServer
{
protected:
    bool m_bRunning;
    std::string m_selfIp;
    int m_selfPort;
    std::string m_uploadPath;
public:
    ZFileServer(const char* self_ip, int port);
    virtual ~ZFileServer();
    
    int setPath(std::string path);
    virtual void start() = 0;
    virtual void stop() = 0;
    
};

class ZUDPFileServer : public ZFileServer
{
private:
    OS_UdpSocket m_sock;
public:
    ZUDPFileServer(const char* self_ip, int port);
    ~ZUDPFileServer();
    void start();
    void stop();
};

class ZTCPFileServer : public ZFileServer
{
    typedef std::vector<void*> VectorThread_t;
private:
    OS_TcpSocket m_sock;
    VectorThread_t m_vecThread;
public:
    ZTCPFileServer(const char* self_ip, int port);
    ~ZTCPFileServer();
    void start();
    void stop();
};

#endif /* fileServer_h */
