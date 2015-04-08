#ifndef RECVBUFFER_H_
#define RECVBUFFER_H_

//////////////////////////////////////////////////////////////////////////
//接受数据用于处理
//////////////////////////////////////////////////////////////////////////
#include "PackBuffer.h"
#include <list>
#include "lock.h"

//接受字符串
class CRecvBuffer
{
public:
	CRecvBuffer()
	{
		InitializeCriticalSection(&m_cs);
	}
	virtual ~CRecvBuffer()
	{
		Clear();
		DeleteCriticalSection(&m_cs);
	}

	//是否为空
	inline bool IsEmpty()			{ return PackBuffetList.empty();}

	//写入字符数据
	inline bool Write(xe_uint8 *pMsg,xe_uint16 nLen)
	{
		//没有消息
		if (pMsg == NULL || nLen >= MAX_BUFFER)
			return false;

		//验证有效包，无效包
		if (nLen <= sizeof(stPackHeader))
			return false;

		//验证crc
		stPackHeader header;
		memcpy(&header,pMsg,sizeof(header));

		if (header.wCrc != MARK_CRC)
			return false;

		stPackBuffer * pPacket = new stPackBuffer();

		memcpy(pPacket->szBuf,pMsg,nLen);

		pPacket->nLen = nLen;					//包总长度




		CSlock cs(&m_cs);
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

		CSlock cs(&m_cs);
		stPackBuffer *pPacket = PackBuffetList.front();
		PackBuffetList.pop_front();

		return pPacket;
	}

	//写入字符数据,自己发送给自己（用于从其他线程到玩家线程）
	inline bool Write(stPackHeader *pHeader, xe_uint8 *pMsg)
	{
		//没有消息
		if (pHeader == NULL || pMsg == NULL)
			return false;

		int nHeadLen = sizeof(stPackHeader);

		//包太长了
		if (pHeader->wSize +  nHeadLen >= MAX_BUFFER)
			return false;

		stPackBuffer * pPacket = new stPackBuffer();

		memcpy(pPacket->szBuf,pHeader,nHeadLen);

		memcpy(pPacket->szBuf + nHeadLen,pMsg,pHeader->wSize);		//获取完整的字符串

		pPacket->nLen = pHeader->wSize + nHeadLen;					//包总长度

		CSlock cs(&m_cs);
		PackBuffetList.push_back(pPacket);

		return true;
	}

	inline bool Write(stPackBuffer *pHeader)
	{
		if (pHeader == NULL)
			return false;

		CSlock cs(&m_cs);
		PackBuffetList.push_back(pHeader);
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
	
	CRITICAL_SECTION m_cs;
};

#endif


