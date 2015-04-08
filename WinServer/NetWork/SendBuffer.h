#ifndef SENDBUFFER_H_
#define SENDBUFFER_H_

#include "PackBuffer.h"
#include <list>

//�����ַ���
class CSendBuffer
{
public:
	CSendBuffer(){};
	virtual ~CSendBuffer()
	{
		Clear();
	}

	//�Ƿ�Ϊ��
	inline bool IsEmpty()			{ return PackBuffetList.empty();}

	//д���ַ�����
	inline bool Write(stPackHeader *pHeader, xe_uint8 *pMsg,xe_uint16 nLen)
	{
		//û����Ϣ
		if (pHeader == NULL || pMsg == NULL)
			return false;

		int nHeadLen = sizeof(stPackHeader);

		//��̫����
		if (nLen +  nHeadLen >= MAX_BUFFER)
			return false;
		
		pHeader->wSize = nLen;

		stPackBuffer * pPacket = new stPackBuffer();

		memcpy(pPacket->szBuf,pHeader,nHeadLen);

		memcpy(pPacket->szBuf + nHeadLen,pMsg,pHeader->wSize);		//��ȡ�������ַ���

		pPacket->nLen = pHeader->wSize + nHeadLen;					//���ܳ���

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

