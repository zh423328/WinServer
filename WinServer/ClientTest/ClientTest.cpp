// ClientTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GlobalDefine.h"

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wd;
	WSAStartup(MAKEWORD(2,2),&wd);

	SOCKET sClient = socket(AF_INET,SOCK_STREAM,0);

	if(sClient == SOCKET_ERROR)
	{
		printf("socket init failed!\n");
		return -1;
	}

	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(DEFAULT_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(DEFAULT_IP);

	xe_uint32 nRet = connect(sClient,(SOCKADDR*)&serverAddr,sizeof(SOCKADDR_IN));
	if (nRet == SOCKET_ERROR)
	{
		printf("socket init failed!\n");
		return -1;
	}

	//·¢ËÍ
	
	char szBuf[2] = "1";

	stPackHeader header;
	memset(&header,0,sizeof(stPackHeader));
	header.wCrc = MARK_CRC;
	header.wSize = 1;

	char szMsg[1024] = {0};

	memcpy(szMsg,&header,sizeof(stPackHeader));

	memcpy(szMsg+sizeof(stPackHeader),szBuf,2);

	int nLen = send(sClient,szMsg,strlen(szMsg),0);
	if (nLen!= 0)
	{
		while(recv(sClient,szMsg,1024,0))
		{
			printf("%s\n",szMsg);
		}
	}


	WSACleanup();
	return 0;
}

