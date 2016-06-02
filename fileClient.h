
// fileClient
#ifndef FILECLIENT_H
#define FILECLIENT_H
#include <string>
#include <vector>
//#include <map>
#include "osapi/socket.h"
#include "filePackage.h"
#include "osapi/Thread.h"
#include "afutil/AfMsgQueue.h"
#define  PORTCOUNT 5; // 定义端口数量

// ZFileClient 基本用法设计：
// ZFileClient* fs = new ZUDPFileClient(self_port); // 如果是TCP，则new ZTCPFileServer(self_port)
// fs->connect(peer_ip, peer_port); // 目的是与Tcp／ip 接口统一
// fs->regeditProgress(...); //注册进度显示函数
// fs->setFileList(...);
// fs->upload();
class ZFileClient;
typedef AfMsgQueue<std::string>  MsgQueue;
// ZSendFileThread定义
class ZSendFileThread : public OS_Thread
{
	OS_UdpSocket m_sock;

	// 下面3个变量都是从其他地方传进来的
	ZFileClient* m_pFileClient;
	MsgQueue& m_quefileURI;
	OS_SockAddr& m_peerAddr;
private:
	std::string getFileName(std::string fileURI);
	int Routine();
public:
	ZSendFileThread(ZFileClient* pFileClient, MsgQueue& quefileURI, int port, OS_SockAddr& peer);
	~ZSendFileThread();
};


class ZFileClient
{
	typedef void (*FOnProgress)(const std::string filename, unsigned long long filelength, unsigned long long currentPossion, void* userData);
protected:
	int m_selfPort;
	OS_SockAddr m_peerAddr;
	MsgQueue m_queFileURI;

	void* m_userData;
public:
	FOnProgress OnProgress;
public:
	ZFileClient(int port) : m_selfPort(port), m_queFileURI(100), OnProgress(NULL)
	{
	}

	void setFileList(std::string fileURIList[], int nCount)
	{
		for (int i = 0; i < nCount; ++i)
		{
			m_queFileURI.push(fileURIList[i]);
		}
		
	}
	void regeditProgress(FOnProgress fun_onProgress, void userData)
	{
		OnProgress = fun_onProgress;
		m_userData = userData;
	}
	virtual bool connect(std::string peer_ip, int peer_port) = 0;
	virtual void upload() = 0;
	virtual void stop() = 0;
};

class ZUDPFileClient : public ZFileClient
{
	typedef std::vector<ZSendFileThread*> VecSendThread;
	VecSendThread m_vecSendTread;
public:
	ZUDPFileClient(int port);
	~ZUDPFileClient();

	bool connect(std::string peer_ip, int peer_port);
	void upload();
	void stop();
};
#endif