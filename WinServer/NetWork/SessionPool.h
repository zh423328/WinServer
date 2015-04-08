#ifndef SESSIONPOOL_H_
#define SESSIONPOOL_H_

#include "GlobalDefine.h"


//���ӳ�
class CSessionList;
class CSession;

class CSessionPool 
{
public:
	CSessionPool(xe_uint32 dwSize);
	~CSessionPool();


	//Ԥ�ȴ����洢
	void Create(xe_uint32 dwSize);

	void CreateStepSize();

	//����һ��session
	CSession* Alloc();

	//�ͷ�
	void	Free(CSession *pSession);

	//ʣ�����session�ɷ���
	xe_uint32 GetLength();

private:
	CSessionList * m_pList;			//������

	xe_uint32	m_nSize;			//����С
	xe_uint32	m_nStepSize;		//1/10
};

#endif