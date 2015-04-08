#ifndef SENDBUFFER_H_
#define SENDBUFFER_H_

#include "PackBuffer.h"
#include <list>

//发送字符串
class CSendBuffer
{
public:
	CSendBuffer(){};
	virtual ~CSendBuffer()
	{
		Clear();
	}

	//是否为空
	inline bool IsEmpty()			{ return PackBuffetList.empty();}

	//写入字符数据
	inline bool Write(stPackHeader *pHeader, xe_uint8 *pMsg,xe_uint16 nLen)
	{
		//没有消息
		if (pHeader == NULL || pMsg == NULL)
			return false;

		int nHeadLen = sizeof(stPackHeader);

		//包太长了
		if (nLen +  nHeadLen >= MAX_BUFFER)
			return false;
		
		pHeader->wSize = nLen;

		stPackBuffer * pPacket = new stPackBuffer();

		memcpy(pPacket->szBuf,pHeader,nHeadLen);

		memcpy(pPacket->szBuf + nHeadLen,pMsg,pHeader->wSize);		//获取完整的字符串

		pPacket->nLen = pHeader->wSize + nHeadLen;					//包总长度

		PackBuffetList.push_back(pPacket);
		
		return true;
	}

	//获取当前还有多少包未发出去
	inline xe_uint16 GetPacketSize()		{ return PackBuffetList.size();}

	//获取第一个包
	inline stPackBuffer * PeerPacket()
	{
		if (IsEmpty())
			return NULL;

		stPackBuffer *pPacket = PackBuffetList.front();
		PackBuffetList.pop_front();

		return pPacket;
	}

	inline bool Clear()
	{
		stPackBuffer *pBuf = NULL;
		for (std::list<stPackBuffer*>::iterator iter = PackBuffetList.begin(); iter != PackBuffetList.end(); ++iter)
		{
			SAFE_DELETE(pBuf);
		}

		PackBuffetList.clear();

		return true;
	}

private:
	std::list<stPackBuffer*> PackBuffetList;
};

#endif


