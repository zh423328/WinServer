#ifndef GLOBAL_DEFINE_H_
#define GLOBAL_DEFINE_H_

#include <winsock2.h>		// ������windows.h֮ǰ������ ��������ģ���������
#include <MSWSock.h>
#include <windows.h> 
#include <stdio.h>
#include <string>
#include <stdlib.h>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

//1�ֽ�
#ifndef xe_int8
#define xe_int8		char
#endif

#ifndef xe_uint8
#define xe_uint8	unsigned char
#endif

//2���ֽ�
#ifndef xe_int16
#define xe_int16	short
#endif

//2���ֽ�
#ifndef xe_uint16
#define xe_uint16	unsigned short
#endif


//4���ֽ�
#ifndef xe_int32
#define xe_int32	int
#endif


#ifndef xe_uint32
#define xe_uint32	unsigned int
#endif

//8�ֽ�
#ifndef xe_int64
#define xe_int64	long long
#endif

#ifndef xe_uint64
#define xe_uint64	unsigned long long
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(p)	{if(p) free(p);p=NULL;}
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)	{if(p) delete p;p=NULL;}
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(ptr) {if(ptr){delete []ptr;ptr=NULL;}}
#endif

// �ͷž����
#ifndef	RELEASE_HANDLE
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}
#endif

// �ͷ�Socket��
#ifndef RELEASE_SOCKET
#define RELEASE_SOCKET(x)               {if(x !=INVALID_SOCKET) { closesocket(x);x=INVALID_SOCKET;}}
#endif

// Ĭ�϶˿�
#define DEFAULT_PORT          12345    
// Ĭ��IP��ַ
#define DEFAULT_IP            "127.0.0.1"


// ͬʱͶ�ݵ�Accept���������(��ͬʱ���ӵ��������),
#define MAX_POST_ACCEPT              100
// ���ݸ�Worker�̵߳��˳��ź�
#define EXIT_CODE                    NULL


#define MARK_CRC		0x1234
#define MAX_BUFFER		4096					//4090(msg len)+sizeof(stPackHeader)

//�ڴ����
#pragma pack(push)
#pragma pack(1)


struct stPackHeader
{
	xe_uint16 wCrc;				//crc
	xe_uint16 wSize;			//���˰�ͷ�Ĵ�С
	xe_uint8  bCmdGroup;		//����Ϣ
	xe_uint8  bCmd;				//����Ϣ
	xe_int32  dwUseKey;			//�����û���Ψһ��ʾ���������������ʱ��dwUseKeyΪ0��������ת������Ϸ����dwUseKeyΪ��������صı�־��
};


struct stPackBuffer
{
	stPackBuffer()
	{
		memset(szBuf,0,MAX_BUFFER);
		nLen = 0;
	}
	~stPackBuffer()
	{

	}

	//���ַ���
	char szBuf[MAX_BUFFER];
	xe_uint16 nLen;
};
#pragma pack(pop)



//���Ĵ�����ֹ������
#ifdef USE_GBK
//�ַ�������
char * xe_strstr(const char* src,const char* szFind);

//�ַ�����
char * xe_strchr(const char* src,int ch);
#endif



#endif