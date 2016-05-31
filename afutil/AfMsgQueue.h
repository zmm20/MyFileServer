#ifndef _AF_MSG_QUEUE_H
#define _AF_MSG_QUEUE_H

/* AfMsgQueue
   内部有锁控制的消息队列。线程安全。

   作者：邵发
   版本：2016-02-02
   最新版本请在官网 http://afanihao.cn 上获取
*/

#include "osapi/Mutex.h"
#include <list>
using std::list;

template <typename _T>
class AfMsgQueue
{
public:
	// 设定最大容量
	AfMsgQueue(int maxLength)
	{
		m_maxLength = maxLength;
		m_count = 0;
	}

	// 存入一个对象
	// 返回1，成功；返回0，已满
	int push(const _T& t)
	{
		int rc = 0;
		m_lock.Lock();
		if(m_values.size() < m_maxLength)
		{
			m_values.push_back(t);
			rc = 1;
			m_count ++;
		}
		m_lock.Unlock();
		return rc;
	}

	// 取出一个对象
	// 返回1，成功；返回0，为空
	int pop(_T& t)
	{
		int rc = 0;
		m_lock.Lock();
		if(m_values.size() > 0)
		{
			t = m_values.front();
			m_values.pop_front();
			rc = 1;
			m_count --;
		}
		m_lock.Unlock();

		return rc;
	}

	// 取出N个对象
	// 返回值：实际取出的个数
	int pop(list<_T>& output, int n)
	{
		int rc = 0;
		m_lock.Lock();
		for(int i=0; i<n; i++)
		{
			if(m_values.size() > 0)
			{
				_T& v = m_values.front();
				output.push_back(v);
				rc += 1;

				m_values.pop_front();				
				m_count --;	
			}
		}
		m_lock.Unlock();
		return rc;
	}

	// 已存入的对象的个数
	int size() const
	{
		return m_count;
	}


private:
	int m_maxLength;
	OS_Mutex m_lock; // 互斥锁
	list<_T> m_values;
	int m_count;
};




#endif


