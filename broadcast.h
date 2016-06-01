//
//  broadcast.h
//  broadcast
//
//  Created by 周满满 on 6/1/16.
//  Copyright © 2016 周满满. All rights reserved.
//

#ifndef Broadcast_h
#define Broadcast_h
#include "osapi/socket.h"
#include "osapi/thread.h"

typedef void (*FOnRecieved)(OS_UdpSocket& sender, std::string ip, int port, const char* buf);

class ZBroadcast : public OS_Thread
{
protected:
    bool m_end;
    int m_selfPort;
    FOnRecieved m_onRecieved;
    OS_UdpSocket m_socket;
public:
    ZBroadcast(int selfPort)
        : OS_Thread(), m_selfPort(selfPort)
        , m_onRecieved(NULL), m_end(false){}
    ~ZBroadcast(){}
    
    void regeditRecieve(FOnRecieved onRecieved)
    {
        m_onRecieved = onRecieved;
    }
    
    int start()
    {
        OS_SockAddr local(m_selfPort);
        int ret = m_socket.Open(local, true);
        
        if (ret == - 1)
            return -1;
        
        ret = m_socket.SetOpt_Broadcast(true);
        if (ret == -1)
            return -1;

        Run();
        return 0;
    }
    void end()
    {
        m_end = true;
        m_socket.Close();
        Join(this);
    }
    
    // 客户端的发送函数
    virtual int SendTo(const void *buf, int len, int broadcastPort) = 0;
};

class ZBroadcastServer : public ZBroadcast
{
    int SendTo(const void *buf, int len, int broadcastPort){return 0;}
public:
    ZBroadcastServer(int broadcastPort) : ZBroadcast(broadcastPort){}
    ~ZBroadcastServer(){}
private:
    int Routine()
    {
        int ret;
        // 广播地址
        OS_SockAddr recievedAddr;
        char buf[1024] = {0};
        while(!m_end)
        {
            memset(buf, 0, 1024);
            //从广播地址接受消息
            ret = m_socket.RecvFrom(buf, 1024, recievedAddr);
            if(ret == -1)
            {
                fprintf(stderr, "RecvFrom(...) error\n");
                return -1;
            }
            else
            {
                if (m_onRecieved)
                {
                    m_onRecieved(m_socket, recievedAddr.GetIp_str(), recievedAddr.GetPort(), buf);
                }
            }
        }

        return 0;
    }
};


class ZBroadcastClient : public ZBroadcast
{
public:
    ZBroadcastClient(int selfPort) : ZBroadcast(selfPort) {}
    ~ZBroadcastClient(){}
    
    int SendTo(const void *buf, int len, int broadcastPort)
    {
        OS_SockAddr broadcastAddr("255.255.255.255", broadcastPort);
        return m_socket.SendTo(buf, len, broadcastAddr);
    }
private:
    int Routine()
    {
        char buf[1024];
        int ret;
        OS_SockAddr recievedAddr;
        while(!m_end)
        {
            ret = m_socket.RecvFrom(buf, 1024, recievedAddr);
            if(ret == - 1)
            {
                fprintf(stderr, "RecvFrom(...) error\n");
                return -1;
            }
            else
            {
                if (m_onRecieved)
                {
                    m_onRecieved(m_socket, recievedAddr.GetIp_str(), recievedAddr.GetPort(), buf);
                }
            }
        }
        
        return 0;
    }
};
#endif /* Broadcast_h */
