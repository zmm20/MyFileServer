// �������mac��
#define _CRT_SECURE_NO_WARNINGS /* VS2013,2015��Ҫ��һ�� */
#include <stdio.h>
#include <string.h>
#include "osapi\Socket.h"
#include "FilePackage.h"
#include "fileClient.h"


void OnProgress(const std::string filename, unsigned long long filelength, unsigned long long currentPossion, void* userData);
int main()
{
	printf("���ͷ�: port=9001 ...\n");
	
	std::string fileList[2] = {"Everything.exe", "Everything.rar"};
	ZFileClient* fc = new ZUDPFileClient(9001);
	fc->setFileList(fileList, 2);
	fc->regeditProgress(OnProgress, NULL);
	fc->connect("192.168.1.101", 9000);

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

		CONSOLE_SCREEN_BUFFER_INFO SBInfo; //���ڴ洢��Ҫ��õĻ��������Ϣ    
		HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);//�����׼���������� 
		GetConsoleScreenBufferInfo(hOut, &SBInfo);//��û����������������˵�ǰ����λ����Ϣ    
		// COORD PrePo;//COORD��һ�������࣬�����Ǻ������������    
		//x = SBInfo.dwCursorPosition.X;//����ȡ������λ��     
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