#ifndef PACKET_BUFFER_H_
#define PACKET_BUFFER_H_

#include "GlobalDefine.h"

#define MARK_CRC		0x1234
#define MAX_BUFFER		4096					//4090(msg len)+sizeof(stPackHeader)

//内存对齐
#pragma pack(push)
#pragma pack(1)


struct stPackHeader
{
	xe_uint16 wCrc;				//crc
	xe_uint16 wSize;			//出了包头的大小
	xe_uint8  bCmdGroup;		//主消息
	xe_uint8  bCmd;				//子消息
	xe_int32  dwUseKey;			//连接用户的唯一标示（当玩家连入网关时，dwUseKey为0，当网关转发到游戏服，dwUseKey为玩家在网关的标志）
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

	//包字符串
	char szBuf[MAX_BUFFER];
	xe_uint16 nLen;
};
#pragma pack(pop)




#endif