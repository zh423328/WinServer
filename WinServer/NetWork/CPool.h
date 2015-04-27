//////////////////////////////////////////////////////////////////////////
//�����
//////////////////////////////////////////////////////////////////////////
#ifndef CPOOL_H_
#define CPOOL_H_

#include "GlobalDefine.h"
#include <list>

template<class T>
class CPool
{
public:
	CPool(int nCount);
	~CPool();

	T* Alloc();				//����
	void Free(T* data);		//���������е��ã�������д��������

	void Create();

	void Step();

	void Purge();			//ɾ�����������

private:
	std::list<T*> m_FreeList;	//�е��б�
	
	xe_uint32 m_nCount;			//��ʼ��
	xe_uint32 m_nStepCount;		//ÿһ��

	CLock m_cs;					//�ٽ���
};


template<class T>
CPool<T>::CPool( int nCount )
{
	m_nCount = nCount;
	m_nStepCount = max(1,m_nStepCount/10);

	Create();
}

template<class T>
CPool<T>::~CPool()
{
	Purge();
}

template<class T>
void CPool<T>::Create()
{
	CAutoLock lock(&m_cs);

	for (int i = 0; i <m_nCount; ++i)
	{
		T *pData = new T;

		m_FreeList.push_back(pData);
	}
}

template<class T>
void CPool<T>::Step()
{
	CAutoLock lock(&m_cs);

	for (int i = 0; i <m_nStepCount; ++i)
	{
		T *pData = new T;

		m_FreeList.push_back(pData);
	}
}

template<class T>
T* CPool<T>::Alloc()
{
	CAutoLock lock(&m_cs);

	if (m_FreeList.empty())
	{
		Step();
	}

	T *pData = m_FreeList.front();
	m_FreeList.pop_front();

	return pData;
}

template<class T>
void CPool<T>::Free(T* data)
{
	CAutoLock lock(&m_cs);

	m_FreeList.push_back(data);
}

//���ɾ��
template<class T>
void CPool<T>::Purge()
{
	T *pData = NULL;
	for (std::list<T*>::iterator iter = m_FreeList.begin(); iter != m_FreeList.end(); ++iter)
	{
		pData = *iter;
		SAFE_DELETE(pData);
	}
}

#endif