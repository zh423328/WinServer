// NetWork.cpp : Defines the entry point for the console application.
///////////////////////////////////////////////////////////////////////////
//����ģ��ܹ�
//////////////////////////////////////////////////////////////////////////



#include "stdafx.h"
#include "TcpServer.h"


int _tmain(int argc, _TCHAR* argv[])
{
	cIocpServer *pServer = new cIocpServer();
	pServer->Start();

	while(pServer->Running())
	{
		pServer->Update();
	}
	return 0;
}

