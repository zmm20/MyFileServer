//
//  fileServer.cpp
//  UDP_HostB
//
//  Created by 周满满 on 5/25/16.
//  Copyright © 2016 周满满. All rights reserved.
//

#include "fileServer.h"
#include "afutil/Endian.h"
#include "osapi/Thread.h"

ZUDPFileServer::ZUDPFileServer(int port):ZFileServer(port)
{
}

ZUDPFileServer::~ZUDPFileServer()
{
}
void ZUDPFileServer::start()
{
    OS_SockAddr local(m_selfPort); // 默认IP：0.0.0.0
    m_sock.Open(local, true);
    
    m_mapNameFile.clear();
    const int BUFSIZE = 1024 * 2;
    unsigned char* buf = new unsigned char[BUFSIZE];
    OS_SockAddr peer; // 对方的地址
    
    int ret;
    int iSucceed = 0;
    std::string strTemp;
    std::string errStr;
    while(1)
    {
        memset(buf, 0, BUFSIZE);
        ret = m_sock.RecvFrom(buf, BUFSIZE, peer);
        if(ret == -1)
        {
            printf("RecvFrom error\n");
            continue;
        }
        
        ZFilePackage pkg;
        pkg.UnSerialize(buf, ret);
        
        // 1表示传输结束
        if (pkg.code == 1)
        {
            MapName_File_t::iterator iFind = m_mapNameFile.find(pkg.filename);
            FILE* fp = iFind->second;
            printf("end %s; peer addr:%s:%d\n", iFind->first.c_str(), peer.GetIp_str().c_str(), peer.GetPort());
            fclose(fp);
            m_mapNameFile.erase(iFind);
            
            continue;
        }
        
        if (pkg.currentpos == 0)
        {
            strTemp = m_uploadPath + pkg.filename;
            printf("recieve begin: %s\n", pkg.filename.c_str());
            
            FILE* fp = fopen(strTemp.c_str(), "wb");
            if (fp == NULL)
            {
                extern int errno;
                iSucceed = -1;
                errStr = errno;
                errStr += " ";
                errStr += strerror(errno);
            }
            else
            {
                m_mapNameFile[pkg.filename] = fp;
                fwrite(pkg.data, 1, pkg.datalength, fp);
            }
        }
        else
        {
            FILE* fp = m_mapNameFile[pkg.filename];
            fwrite(pkg.data, 1, pkg.datalength, fp);
        }
        
        // 响应客户端
        pkg.code = iSucceed;
        pkg.msg  = errStr;
        pkg.datalength = 0;
        pkg.data = NULL;
        pkg.Serialize();
        ret  = m_sock.SendTo(pkg.GetBuffer(), pkg.GetSize(), peer); // 响应客户端
        if(ret == -1)
        {
            printf("SendTo error\n");
            continue;
        }
    }
    
    delete[] buf;
}

void ZUDPFileServer::stop()
{
    // 关闭socket
    m_sock.Close();
}

/*********************TCP/IP*****************************************/

/* TcpConn:
 用一个线程来维护Client - WorkingSocket之间的通话连接
 */
class TcpConn : public OS_Thread
{
private:
    OS_SockAddr m_peer;
    OS_TcpSocket m_WorkSock;
    std::string m_uploadPath;
public:
    MapName_File_t m_mapNameFile;
public:
    TcpConn(OS_TcpSocket work_sock, std::string& uploadPathRoot, OS_SockAddr& peerAddr)
        : OS_Thread(), m_WorkSock(work_sock), m_uploadPath(uploadPathRoot)
        , m_peer(peerAddr){}
    ~TcpConn(){}
    
private:
    virtual int Routine()
    {
        const int BUFSIZE = 1024 * 2;
        unsigned char *buf = new unsigned char[BUFSIZE];
        
        int ret;
        int iSucceed = 0;
        int realCount;
        std::string strTemp;
        std::string errStr;
        
        m_mapNameFile.clear();
        while(1)
        {
            memset(buf, 0, BUFSIZE);
            
            ret = m_WorkSock.WaitBytes(buf, 4, realCount);
            if(ret == -1)
            {
                printf("Recv error\n");
                break;
            }
            else if (ret == 0)
            {
                printf("client exit\n");
                break;
            }
            
            unsigned int size = btoi_32be(buf);
            
            ret = m_WorkSock.WaitBytes(buf, size, realCount);
            if(ret == -1)
            {
                printf("Recv error\n");
                break;
            }
            else if (ret == 0)
            {
                printf("client exit\n");
                break;
            }
            
            ZFilePackage pkg(false);
            pkg.UnSerialize(buf, ret);
            
            // 1表示传输结束
            if (pkg.code == 1)
            {
                MapName_File_t::iterator iFind = m_mapNameFile.find(pkg.filename);
                FILE* fp = iFind->second;
                printf("end %s\n", iFind->first.c_str());
                fclose(fp);
                m_mapNameFile.erase(iFind);
                
                continue;
            }
            
            if (pkg.currentpos == 0)
            {
                strTemp = m_uploadPath + pkg.filename;
                printf("recieve begin: %s\n", pkg.filename.c_str());
                
                FILE* fp = fopen(strTemp.c_str(), "wb");
                if (fp == NULL)
                {
                    extern int errno;
                    iSucceed = -1;
                    errStr = errno;
                    errStr += " ";
                    errStr += strerror(errno);
                }
                else
                {
                    m_mapNameFile[pkg.filename] = fp;
                    fwrite(pkg.data, 1, pkg.datalength, fp);
                }
            }
            else
            {
                FILE* fp = m_mapNameFile[pkg.filename];
                fwrite(pkg.data, 1, pkg.datalength, fp);
            }
            
            // 响应客户端
            pkg.code = iSucceed;
            pkg.msg  = errStr;
            pkg.datalength = 0;
            pkg.data = NULL;
            pkg.Serialize();
            ret  = m_WorkSock.Send(pkg.GetBuffer(), pkg.GetSize()); // 响应客户端
            if(ret == -1)
            {
                printf("Send error\n");
                break;
            }
        }
        
        delete[] buf;
        m_WorkSock.Close();
        return 0;
    }
};


ZTCPFileServer::ZTCPFileServer(int port) : ZFileServer(port)
{
    m_vecThread.clear();
}
ZTCPFileServer::~ZTCPFileServer()
{
    
}
void ZTCPFileServer::start()
{
    // (1): 占用端口号
    OS_SockAddr local(m_selfPort); // 默认IP：0.0.0.0
    int ret = m_sock.Open(local, true);
    if (ret == -1)
    {
        printf("socket open error\n");
        return;
    }
    // (2): 监听
    ret = m_sock.Listen();
    if (ret == -1)
    {
        printf("socket Listen error\n");
        return;
    }
    
    while(1)
    {
        OS_TcpSocket work_sock;
        OS_SockAddr peerAddr;
        if(m_sock.Accept(&work_sock, &peerAddr) < 0)
        {
            break;
        }
        printf("connect comming, address: %s:%d\n", peerAddr.GetIp_str().c_str(), peerAddr.GetPort());
        
        // 新建一个线程，处理该client的请求
        TcpConn* conn = new TcpConn(work_sock, m_uploadPath, peerAddr);
        conn->Run();
        m_vecThread.push_back(conn);
    }
}
void ZTCPFileServer::stop()
{
    TcpConn* conn;
    for (int i = 0; i < m_vecThread.size(); ++i)
    {
        conn = (TcpConn*)m_vecThread[i];
        OS_Thread::Join(conn);
        delete conn;
    }
    m_vecThread.clear();
    // 关闭socket
    m_sock.Close();
}