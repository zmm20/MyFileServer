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
//ZFileClient* fc = new ZUDPFileServer(self_port); // 如果是TCP，则new ZTCPFileClient(self_port)
//fc->setPath(...); // 设置路径
//fc->start();
typedef std::map<std::string, FILE*> MapName_File_t;

class ZFileServer
{
protected:
    bool m_bRunning;
    int m_selfPort;
    std::string m_uploadPath;
public:
    ZFileServer(int port) : m_selfPort(port), m_uploadPath(""){}
    virtual ~ZFileServer(){}
    
    void setPath(std::string path)
    {
        // 可以做些检查，如果路径不存在，则创建 或提示
        m_uploadPath = path;
    }
    virtual void start() = 0;
    virtual void stop() = 0;
    
};

class ZUDPFileServer : public ZFileServer
{
private:
    OS_UdpSocket m_sock;
    MapName_File_t m_mapNameFile;
public:
    ZUDPFileServer(int port);
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
    ZTCPFileServer(int port);
    ~ZTCPFileServer();
    void start();
    void stop();
};

#endif /* fileServer_h */
