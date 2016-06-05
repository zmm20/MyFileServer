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
#include "fileManage_s.h"

ZFileServer::ZFileServer(int port) : m_selfPort(port), m_uploadPath("")
{
}
ZFileServer::~ZFileServer()
{
    ZFileManage* pFileMange = ZFileManage::getInstance();
    pFileMange->destroy();
}
int ZFileServer::setPath(std::string path)
{
    ZFileManage* pFileMange = ZFileManage::getInstance();
    return pFileMange->setRootDirOfUpload(path);
}

/********** udp file server *********************************/

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
    
    const int BUFSIZE = 1024 * 2;
    unsigned char* buf = new unsigned char[BUFSIZE];
    OS_SockAddr peer; // 对方的地址
    
    int ret;
    int iSucceed = 0;
    std::string strTemp;
    std::string errStr;
    ZFileManage* pFileMange = ZFileManage::getInstance();
    while(1)
    {
        memset(buf, 0, BUFSIZE);
        ret = m_sock.RecvFrom(buf, BUFSIZE, peer);
        if(ret == -1)
        {
            printf("RecvFrom error\n");
            break;
        }
        
        ZFilePackage pkg;
        pkg.UnSerialize(buf, ret);
        
        // 1表示传输结束
        if (pkg.code == 1)
        {
            pFileMange->endWrite(pkg.filename);
            
            printf("end %s; peer addr:%s:%d\n", pkg.filename.c_str(), peer.GetIp_str().c_str(), peer.GetPort());
            continue;
        }
        else
        {
            iSucceed = pFileMange->write(pkg.filename, pkg.currentpos, pkg.data, pkg.datalength, errStr);
            
            // 响应客户端
            pkg.code = iSucceed;
            pkg.msg  = errStr;
            pkg.datalength = 0;
            pkg.data = NULL;
            pkg.Serialize();
            ret  = m_sock.SendTo(pkg.GetBuffer(), pkg.GetSize(), peer);
            if(ret == -1)
            {
                printf("SendTo error\n");
                break;
            }
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
public:
    TcpConn(OS_TcpSocket work_sock, OS_SockAddr& peerAddr)
        : OS_Thread(), m_WorkSock(work_sock), m_peer(peerAddr){}
    ~TcpConn(){}
    
private:
    virtual int Routine()
    {
        const int BUFSIZE = 1024 * 2;
        unsigned char *buf = new unsigned char[BUFSIZE];
        
        int ret;
        int iSucceed = 0;
        int realCount;
        std::string errStr;
        
        ZFileManage* pFileMange = ZFileManage::getInstance();
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
                pFileMange->endWrite(pkg.filename);
                printf("end %s\n", pkg.filename.c_str());
                
                continue;
            }
            else
            {
                iSucceed = pFileMange->write(pkg.filename, pkg.currentpos, pkg.data, pkg.datalength, errStr);
                
                // 响应客户端
                pkg.code = iSucceed;
                pkg.msg  = errStr;
                pkg.datalength = 0;
                pkg.data = NULL;
                pkg.Serialize();
                ret  = m_WorkSock.Send(pkg.GetBuffer(), pkg.GetSize());
                if(ret == -1)
                {
                    printf("Send error\n");
                    break;
                }
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
        TcpConn* conn = new TcpConn(work_sock, peerAddr);
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