#ifndef PACKET_BUFFER_H_
#define PACKET_BUFFER_H_

#include "GlobalDefine.h"

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




#endif