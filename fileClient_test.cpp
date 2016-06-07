// 服务端在mac上
#define _CRT_SECURE_NO_WARNINGS /* VS2013,2015需要这一行 */
#include <stdio.h>
#include <string.h>
#include "fileClient.h"
#include "broadcast.h"
#define  SERVER_IP	"192.168.1.101"
#define  ANY_IP		"0.0.0.0"

void OnProgress(const std::string filename, unsigned long long filelength, unsigned long long currentPossion, void* userData);


int main()
{
	bool isUdp = true;
	std::string selfIp = ANY_IP;
	printf("发送方: ip = %s port = 9001 protocal = %s...\n", selfIp.c_str(), isUdp ? "udp" : "tcp/ip");
	
	
	ZFileClient* fc;
	if (isUdp)
		fc = new ZUDPFileClient(9001);
	else
		fc = new ZTCPFileClient(9001);

	std::string fileList[2] = {"Everything.exe", "Everything.rar"};
	fc->setFileList(fileList, 2);
	fc->regeditProgress(OnProgress, NULL);
	fc->connect(SERVER_IP, 9000);

	printf("please enter return key to start");
	getchar();
	fc->upload();
	
	getchar();
	return 0;
}
int x = 25, y;
void OnProgress(const std::string filename, unsigned long long filelength, unsigned long long currentPossion, void* userData)
{
	static bool bFirst = true;
	if (bFirst)
	{
		printf("Everything.exe progress = \n");
		printf("Everything.rar progress = ");

		CONSOLE_SCREEN_BUFFER_INFO SBInfo; //用于存储将要获得的环境句柄信息    
		HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);//保存标准输出环境句柄 
		GetConsoleScreenBufferInfo(hOut, &SBInfo);//获得环境句柄，里面包含了当前光标的位置信息    
		// COORD PrePo;//COORD是一个坐标类，里面是横坐标和纵坐标    
		//x = SBInfo.dwCursorPosition.X;//从中取出光标的位置     
		y = SBInfo.dwCursorPosition.Y;    

	}
 	bFirst = false;

	COORD pt;	
	//setbuf(stdout, NULL);
	float percent = 1.0 * currentPossion / filelength * 100;
	if (filename == "Everything.exe")
	{
		pt.X = x;
		pt.Y = y - 1;	
		
	}
	else
	{	
		pt.X = x;
		pt.Y = y;
	}
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),pt);
	printf("%0.2f", percent);

}