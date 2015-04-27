#ifndef RECVBUFFER_H_
#define RECVBUFFER_H_

//////////////////////////////////////////////////////////////////////////
//�����������ڴ���
//////////////////////////////////////////////////////////////////////////
#include "PackBuffer.h"
#include <list>
#include "lock.h"

//�����ַ���
class CRecvBuffer
{
public:
	CRecvBuffer()
	{
		//InitializeCriticalSection(&m_cs);
	}
	virtual ~CRecvBuffer()
	{
		Clear();
		//DeleteCriticalSection(&m_cs);
	}

	//�Ƿ�Ϊ��
	inline bool IsEmpty()			{ return PackBuffetList.empty();}

	//д���ַ�����
	inline bool Write(xe_uint8 *pMsg,xe_uint16 nLen)
	{
		//û����Ϣ
		if (pMsg == NULL || nLen >= MAX_BUFFER)
			return false;

		//��֤��Ч������Ч��
		if (nLen <= sizeof(stPackHeader))
			return false;

		//��֤crc
		stPackHeader header;
		memcpy(&header,pMsg,sizeof(header));

		if (header.wCrc != MARK_CRC)
			return false;

		stPackBuffer * pPacket = new stPackBuffer();

		memcpy(pPacket->szBuf,pMsg,nLen);

		pPacket->nLen = nLen;					//���ܳ���




		CAutoLock cs(&m_cs);
		PackBuffetList.push_back(pPacket);

		return true;
	}

	//��ȡ��ǰ���ж��ٰ�δ����ȥ
	inline xe_uint16 GetPacketSize()		{ return PackBuffetList.size();}

	//��ȡ��һ����
	inline stPackBuffer * PeerPacket()
	{
		if (IsEmpty())
			return NULL;

		CAutoLock cs(&m_cs);
		stPackBuffer *pPacket = PackBuffetList.front();
		PackBuffetList.pop_front();

		return pPacket;
	}

	//д���ַ�����,�Լ����͸��Լ������ڴ������̵߳�����̣߳�
	inline bool Write(stPackHeader *pHeader, xe_uint8 *pMsg)
	{
		//û����Ϣ
		if (pHeader == NULL || pMsg == NULL)
			return false;

		int nHeadLen = sizeof(stPackHeader);

		//��̫����
		if (pHeader->wSize +  nHeadLen >= MAX_BUFFER)
			return false;

		stPackBuffer * pPacket = new stPackBuffer();

		memcpy(pPacket->szBuf,pHeader,nHeadLen);

		memcpy(pPacket->szBuf + nHeadLen,pMsg,pHeader->wSize);		//��ȡ�������ַ���

		pPacket->nLen = pHeader->wSize + nHeadLen;					//���ܳ���

		CAutoLock cs(&m_cs);
		PackBuffetList.push_back(pPacket);

		return true;
	}

	inline bool Write(stPackBuffer *pHeader)
	{
		if (pHeader == NULL)
			return false;

		CAutoLock cs(&m_cs);
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
	
	CLock m_cs;
};

#endif


