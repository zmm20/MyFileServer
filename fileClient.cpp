#include "fileClient.h"
#ifdef _WIN32
#define SocketErrNo WSAGetLastError()
#else
#define SocketErrNo errno
#endif

OS_SockAddr ZSendFileThread::PeerAddr;
OS_Mutex ZSendFileThread::m_mutex;
std::string ZSendFileThread::getFileName(std::string fileURI)
{
	for (int i = fileURI.length() - 1; i >=0; --i)
	{
		if (fileURI[i] == '\\' || fileURI[i] == '/')
		{
			fileURI = fileURI.substr(i + 1, fileURI.length());
			break;
		}
	}
	return fileURI;
}

int ZSendFileThread::Routine()
{
	bool bFirst = true;

	FILE* pFile = NULL;
	std::string fileURI;
	std::string filename;

	const int DATAMAX = 1024;
	unsigned long long fileLength;
	//之前是声明成静态数组的， 但是程序不定时的报错， 改成动态就没问题了。我想可能是局部变量存储在栈区，而栈区声请的内存太大会溢出（会有5个线程同时运行）
	//经查找资料， 操作系统的栈一般就1～4M而已
	unsigned char *buf = new unsigned char[DATAMAX]; 

	ZFilePackage pkg;
	int rt;
	unsigned long long trackProgree = 0;
	OS_SockAddr recvAddr;
	while(!m_bEnd)
	{
		if (bFirst)
		{
			rt = m_quefileURI.pop(fileURI);
			if (rt == 0) // 如果队列中的文件个数为0，则结束循环
				break;
			filename = getFileName(fileURI);
			pFile = fopen(fileURI.c_str(), "rb");

			// 文件大小
			fseek(pFile, 0, SEEK_END);
			fileLength = ftell(pFile);
			fseek(pFile, 0, SEEK_SET);

			printf("begin file: %s\n", filename.c_str());
		}

		pkg.msgId = 0;
		pkg.code = 0;
		pkg.msg = "";
		pkg.cmd = ZFilePackage::UPDATE;
		pkg.filelength = fileLength;
		pkg.currentpos = ftell(pFile);
		pkg.filename = filename.c_str();

		if (m_pFileClient->OnProgress)
		{
			if (1.0 * (pkg.currentpos - trackProgree) / fileLength * 100 > 1.0
				|| pkg.currentpos == fileLength)
			{// 当完成大于1%时候才调用一次进度，否则太频繁了
                m_mutex.Lock();// 之所以加锁，是因为用户通常会在这个回调函数中控制进度条，那么这个进度条就在多线程中共享了
				m_pFileClient->OnProgress(filename, fileLength, pkg.currentpos, m_pFileClient->m_userData);
                m_mutex.Unlock();

				trackProgree = pkg.currentpos;
			}

		}
		memset(buf, 0, DATAMAX);
		int n = fread(buf, 1, DATAMAX, pFile);
		if (n == 0)
		{// 说明已经完成一个文件的上传
			pkg.code = 1;
			pkg.datalength = 0;
			pkg.Serialize();
			rt = m_sock.SendTo(pkg.GetBuffer(), pkg.GetSize(), PeerAddr);
			if (rt == -1)
			{
				printf("SendTo error: %d\n", SocketErrNo);
				fclose(pFile);
				pFile = NULL;
				break;
			}

			fclose(pFile);
			pFile = NULL;

			// 开始下一个文件
			bFirst = true;
			OS_Thread::Msleep(10); // 减少丢包概率
			continue;
		}
		else
		{
			bFirst = false;
		}

		pkg.data = buf;
		pkg.datalength = n;

		// 发送
		pkg.Serialize();
		rt = m_sock.SendTo(pkg.GetBuffer(), pkg.GetSize(), PeerAddr);
		if (rt == -1)
		{
			printf("SendTo error: %d\n", SocketErrNo);
			fclose(pFile);
			pFile = NULL;
			break;
		}

		n = m_sock.RecvFrom(buf, DATAMAX, recvAddr);
		if (n == -1)
		{
			printf("RecvFrom error: %d\n", SocketErrNo);
			break;
		}
		else
		{
			if (pkg.UnSerialize(buf, n) && pkg.code == -1)
			{// 说明当前上传文件发生错误
				fclose(pFile);
				pFile = NULL;

				bFirst = true; // 开始新的文件上传
				printf("error code = %d, msg = %s\n", pkg.code, pkg.msg.c_str());
				continue;
			}
		}

	}

	delete[] buf;

	if (pFile)
		fclose(pFile);

	return 0;
}


ZUDPFileClient::ZUDPFileClient(const char* ip, int port) : ZFileClient(ip, port)
{
	for (int i = 0; i < 5; ++i)
	{// 准备5个线程对象
		ZSendFileThread* sendThread = new ZSendFileThread(this, m_queFileURI, port + i);
		m_vecSendTread.push_back(sendThread);
	}
}
ZUDPFileClient::~ZUDPFileClient()
{
	ZSendFileThread* sendThread;
	for (int i = 0; i < 5; ++i)
	{
		sendThread = m_vecSendTread[i];
		delete sendThread;
	}
	m_vecSendTread.clear();
}

bool ZUDPFileClient::connect(std::string peer_ip, int peer_port)
{
	m_peerAddr = OS_SockAddr(peer_ip.c_str(), peer_port);
	ZSendFileThread::PeerAddr = m_peerAddr;

	return true;
}
void ZUDPFileClient::upload()
{
	ZSendFileThread* sendThread;

	int fileCount = m_queFileURI.size();
	if (fileCount > 5)
		fileCount = 5;
	for (int i = 0; i < fileCount; ++i)
	{
		sendThread = m_vecSendTread[i];

		sendThread->start();
	}
}
void ZUDPFileClient::stop()
{
	ZSendFileThread* sendThread;
	for (int i = 0; i < 5; ++i)
	{
		sendThread = m_vecSendTread[i];
		sendThread->end();
	}
}


ZTCPFileClient::ZTCPFileClient(const char* ip, int port) : ZFileClient(ip, port)
{
	OS_SockAddr local(m_selfPort);
	m_sock.Open(local, true);
}

ZTCPFileClient::~ZTCPFileClient()
{
	//	
}

bool ZTCPFileClient::connect(std::string peer_ip, int peer_port)
{
	m_peerAddr = OS_SockAddr(peer_ip.c_str(), peer_port);

	if(	m_sock.Connect(m_peerAddr) < 0)
	{
		printf("无法连接服务器!\n");
		return false;
	}
	return true;
}
void ZTCPFileClient::upload()
{
	bool bFirst = true;

	FILE* pFile = NULL;
	std::string fileURI;
	std::string filename;

	const int DATAMAX = 1024;
	unsigned long long fileLength;
	//之前是声明成静态数组的， 但是程序不定时的报错， 改成动态就没问题了。我想可能是局部变量存储在栈区，而栈区声请的内存太大会溢出（会有5个线程同时运行）
	//经查找资料， 操作系统的栈一般就1～4M而已
	unsigned char *buf = new unsigned char[DATAMAX]; 

	ZFilePackage pkg(false);
	int rt;
	int realCount;
	unsigned long long trackProgree = 0;
	OS_SockAddr recvAddr;
	while(1)
	{
		if (bFirst)
		{
			rt = m_queFileURI.pop(fileURI);
			if (rt == 0) // 如果队列中的文件个数为0，则结束循环
				break;
			filename = getFileName(fileURI);
			pFile = fopen(fileURI.c_str(), "rb");

			// 文件大小
			fseek(pFile, 0, SEEK_END);
			fileLength = ftell(pFile);
			fseek(pFile, 0, SEEK_SET);

			printf("begin file: %s\n", filename.c_str());
		}

		pkg.msgId = 0;
		pkg.code = 0;
		pkg.msg = "";
		pkg.cmd = ZFilePackage::UPDATE;
		pkg.filelength = fileLength;
		pkg.currentpos = ftell(pFile);
		pkg.filename = filename.c_str();

		if (this->OnProgress)
		{
			if (1.0 * (pkg.currentpos - trackProgree) / fileLength * 100 > 1.0
				|| pkg.currentpos == fileLength)
			{// 当完成大于1%时候才调用一次进度，否则太频繁了
				this->OnProgress(filename, fileLength, pkg.currentpos, this->m_userData);
				trackProgree = pkg.currentpos;
			}

		}
		memset(buf, 0, DATAMAX);
		realCount = fread(buf, 1, DATAMAX, pFile);
		if (realCount == 0)
		{// 说明已经完成一个文件的上传
			pkg.code = 1;
			pkg.datalength = 0;
			pkg.Serialize();
			rt = m_sock.Send(pkg.GetBuffer(), pkg.GetSize());
			if (rt == -1)
			{
				printf("SendTo error: %d\n", SocketErrNo);
				fclose(pFile);
				pFile = NULL;
				break;
			}

			fclose(pFile);
			pFile = NULL;

			// 开始下一个文件
			bFirst = true;
			continue;
		}
		else
		{
			bFirst = false;
		}

		pkg.data = buf;
		pkg.datalength = realCount;

		// 发送
		pkg.Serialize();
		rt = m_sock.Send(pkg.GetBuffer(), pkg.GetSize());
		if (rt == -1)
		{
			printf("SendTo error: %d\n", SocketErrNo);
			fclose(pFile);
			pFile = NULL;
			break;
		}

		rt = m_sock.WaitBytes(buf, 4, realCount);
		unsigned int size = btoi_32be(buf);
		rt = m_sock.WaitBytes(buf, size, realCount);
		if (rt == -1)
		{
			printf("RecvFrom error: %d\n", SocketErrNo);
			break;
		}
		else
		{
			if (pkg.UnSerialize(buf, size) && pkg.code == -1)
			{// 说明当前上传文件发生错误
				fclose(pFile);
				pFile = NULL;

				bFirst = true; // 开始新的文件上传
				printf("error code = %d, msg = %s\n", pkg.code, pkg.msg.c_str());
				continue;
			}
		}

	}

	delete[] buf;

	if (pFile)
		fclose(pFile);

	m_sock.Close();

}
void ZTCPFileClient::stop()
{
	m_sock.Close();
}

std::string ZTCPFileClient::getFileName(std::string fileURI)
{
	for (int i = fileURI.length() - 1; i >=0; --i)
	{
		if (fileURI[i] == '\\' || fileURI[i] == '/')
		{
			fileURI = fileURI.substr(i + 1, fileURI.length());
			break;
		}
	}
	return fileURI;
}
