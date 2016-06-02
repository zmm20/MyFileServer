#include "fileClient.h"


ZSendFileThread::ZSendFileThread(ZFileClient* pFileClient, MsgQueue& quefileURI, int port, OS_SockAddr& peer)
	: m_pFileClient(pFileClient), m_quefileURI(quefileURI), m_peerAddr(peer)
{
	OS_SockAddr local(port);// 默认地址：0.0.0.0
	m_sock.Open(local, true);
}

ZSendFileThread::~ZSendFileThread()
{
	m_sock.Close();
}

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
	unsigned char buf[DATAMAX];

	ZFilePackage pkg;
	int rt;
	unsigned long long trackProgree = 0;
	OS_SockAddr recvAddr;
	while(1)
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
				m_pFileClient->OnProgress(filename, fileLength, pkg.currentpos, m_userData);
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
			m_sock.SendTo(pkg.GetBuffer(), pkg.GetSize(), m_peerAddr);
			fclose(pFile);

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
		m_sock.SendTo(pkg.GetBuffer(), pkg.GetSize(), m_peerAddr);

		n = m_sock.RecvFrom(buf, DATAMAX, recvAddr);
		if (n < 0)
		{
#ifdef _WIN32
			printf("error: %d\n", WSAGetLastError());
#else
            printf("error: %d\n", errno);
#endif
			break;
		}
		else
		{
			if (pkg.UnSerialize(buf, n) && pkg.code == -1)
			{// 说明当前上传文件发生错误
				fclose(pFile);

				bFirst = true; // 开始新的文件上传
				printf("error code = %d, msg = %s\n", pkg.code, pkg.msg.c_str());
				continue;
			}
		}
		
	}
	
	return 0;
}


ZUDPFileClient::ZUDPFileClient(int port) : ZFileClient(port)
{
	for (int i = 0; i < 5; ++i)
	{// 准备5个线程对象
		ZSendFileThread* sendThread = new ZSendFileThread(this, m_queFileURI, port + i, m_peerAddr);
		m_vecSendTread.push_back(sendThread);
	}
}
ZUDPFileClient::~ZUDPFileClient()
{
	ZSendFileThread* sendThread;
	for (int i = 0; i < 5; ++i)
	{
		sendThread = m_vecSendTread[0];
		OS_Thread::Join(sendThread);
	}
}

bool ZUDPFileClient::connect(std::string peer_ip, int peer_port)
{
	m_peerAddr = OS_SockAddr(peer_ip.c_str(), peer_port);
	return true;
}
void ZUDPFileClient::upload()
{
	ZSendFileThread* sendThread;

	int fileCount = m_queFileURI.size();
	for (int i = 0; i < fileCount; ++i)
	{
		sendThread = m_vecSendTread[i];

		sendThread->Run();
	}
}
void ZUDPFileClient::stop()
{
}