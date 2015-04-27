//////////////////////////////////////////////////////////////////////////
//缓冲池
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

	T* Alloc();				//分配
	void Free(T* data);		//析构函数中调用，可以重写析构函数

	void Create();

	void Step();

	void Purge();			//删除里面的内容

private:
	std::list<T*> m_FreeList;	//有的列表
	
	xe_uint32 m_nCount;			//初始化
	xe_uint32 m_nStepCount;		//每一步

	CLock m_cs;					//临界区
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

//最后删除
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