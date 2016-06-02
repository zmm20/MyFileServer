//
//  fileServer.cpp
//  UDP_HostB
//
//  Created by 周满满 on 5/25/16.
//  Copyright © 2016 周满满. All rights reserved.
//

#include "fileServer.h"

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
    unsigned char buf[BUFSIZE];
    OS_SockAddr peer; // 对方的地址
    
    int iSucceed = 0;
    std::string strTemp;
    std::string errStr;
    while(1)
    {
        memset(buf, 0, BUFSIZE);
        int n = m_sock.RecvFrom(buf, BUFSIZE, peer);
        if(n <= 0)
        {
            continue;
        }
        
        ZFilePackage pkg;
        pkg.UnSerialize(buf, n);
        
        // 1表示传输结束
        if (pkg.code == 1)
        {
            MapName_File::iterator iFind = m_mapNameFile.find(pkg.filename);
            FILE* fp = iFind->second;
            fclose(fp);
            m_mapNameFile.erase(iFind);
            
            continue;
        }
        
        if (pkg.currentpos == 0)
        {
            strTemp = m_uploadPath + pkg.filename;
            printf("%s\n", strTemp.c_str());
            
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
        m_sock.SendTo(pkg.GetBuffer(), pkg.GetSize(), peer); // 响应客户端
    }
}

void ZUDPFileServer::stop()
{
    // 关闭socket
    m_sock.Close();
}