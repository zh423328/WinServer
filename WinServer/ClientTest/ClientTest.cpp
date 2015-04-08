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



	WSACleanup();
	return 0;
}

