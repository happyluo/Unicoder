// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_SHAREDPTR_H
#define UTIL_SHAREDPTR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <algorithm>
#include <Util/Exception.h>

namespace Util
{
// class SharedPtrBase
///����ʽ���ü�������ָ����࣬ʵ�ֶԱ�����ָ��Ļ�������
template<typename T>
class SharedPtrBase
{
public:
	
	/// ����Ԫ�����ͱ���
	typedef T element_type;
	
	///��ȡ�����ָ��
	T* Get() const
	{
		return m_ptr;
	}
	
	///��ȡ�����ָ��
	T* operator->() const
	{
		if (!m_ptr)
		{
			ThrowNullSharedPtrException(__FILE__, __LINE__);           
		}
		
		return m_ptr;
	}
	
	///��ȡ�������ָ������Ӧ�Ķ���
	T& operator*() const
	{
		if (!m_ptr)
		{
			ThrowNullSharedPtrException(__FILE__, __LINE__);           
		}
		
		return *m_ptr;
	}
	
	///��⵱ǰ�����Ƿ���Ч
	operator bool() const
	{
		return m_ptr ? true : false;
	}
	
	///������������ָ��
	void Swap(SharedPtrBase& other)
	{
		std::swap(m_ptr, other.m_ptr);
	}
	
	///�������ָ��
	T *m_ptr;				

private:
	
	///�׳���ָ���쳣
	///@param	file		�����쳣���ļ�
	///@param	line	�쳣����������
	void ThrowNullSharedPtrException(const char *file, int line) const;
};
	
template<typename T> inline void 
SharedPtrBase<T>::ThrowNullSharedPtrException(const char* file, int line) const
{
	throw NullSharedPtrException(file, line);
}

///�ж�����ָ���Ƿ����
template<typename T, typename U>
inline bool operator ==(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
	T* l = lhs.Get();
	U* r = rhs.Get();
	if (l && r)
	{
		return *l == *r;
	}
	
	return !l && !r;
}

///�ж�����ָ���Ƿ����
template<typename T, typename U>
inline bool operator !=(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
	return !operator ==(lhs, rhs);
}

///�ж���������Ƿ�С���Ҳ�����
template<typename T, typename U>
inline bool operator <(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
	T* l = lhs.Get();
	U* r = rhs.Get();
	if (l && r)
	{
		return *l < *r;
	}
	
	return !l && r;
}

///�ж���������Ƿ�С�ڵ����Ҳ�����
template<typename T, typename U>
inline bool operator <=(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
	return lhs < rhs || lhs == rhs;
}

///�ж���������Ƿ�����Ҳ�����
template<typename T, typename U>
inline bool operator >(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
	return !(lhs < rhs || lhs == rhs);
}

///�ж���������Ƿ���ڵ����Ҳ�����
template<typename T, typename U>
inline bool operator >=(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
	return !(lhs < rhs);
}
	
// class SharedPtr
///����ʽ���ü�������ָ���࣬ʵ�ֶ�ָ�뱾��������Ӧ�ڴ�Ĺ���
template<typename T>
class SharedPtr : public SharedPtrBase<T>
{
public:   
	///���캯��
	///@param	p	ϣ��������Ķ���ָ��
	SharedPtr(T* p = 0)
	{
		this->m_ptr = p;
		
		if (this->m_ptr)
		{
			this->m_ptr->IncRef();
		}
	}

	///�������캯��
	template<typename Y>
	SharedPtr(const SharedPtr<Y>& rhs)
	{
		this->m_ptr = rhs.m_ptr;
		
		if (this->m_ptr)
		{
			this->m_ptr->IncRef();
		}
	}
	
	///�������캯��
	SharedPtr(const SharedPtr& rhs)
	{
		this->m_ptr = rhs.m_ptr;
		
		if (this->m_ptr)
		{
			this->m_ptr->IncRef();
		}
	}
	
	///��������
	~SharedPtr()
	{
		if (this->m_ptr)
		{			
			this->m_ptr->DecRef();
		}
		
		this->m_ptr = 0;
	}

	///��ֵ������
	SharedPtr& operator=(T* p)
	{
		if (this->m_ptr != p)
		{
			if (p)
			{
				p->IncRef();
			}
			
			T* ptr = this->m_ptr;
			this->m_ptr = p;
			
			if (ptr)
			{
				long lRefCount = ptr->GetRef();
				
				ptr->DecRef();
				
				if (1 == lRefCount)
				{
					ptr = 0;
				}
			}
		}
		return *this;
	}
	
	///��ֵ������
	template<typename Y>
	SharedPtr& operator=(const SharedPtr<Y>& rhs)
	{
		if (this->m_ptr != rhs.m_ptr)
		{
			if (rhs.m_ptr)
			{
				rhs.m_ptr->IncRef();
			}
			
			T* ptr = this->m_ptr;
			this->m_ptr = rhs.m_ptr;
			
			if (ptr)
			{
				long lRefCount = ptr->GetRef();
				
				ptr->DecRef();
				
				if (1 == lRefCount)
				{
					ptr = 0;
				}
			}
		}
		return *this;
	}
	
	///��ֵ������
	SharedPtr& operator=(const SharedPtr& rhs)
	{
		if (this->m_ptr != rhs.m_ptr)
		{
			if (rhs.m_ptr)
			{
				rhs.m_ptr->IncRef();
			}
			
			T* ptr = this->m_ptr;
			this->m_ptr = rhs.m_ptr;
			
			if (ptr)
			{
				long lRefCount = ptr->GetRef();
				
				ptr->DecRef();
				
				if (1 == lRefCount)
				{
					ptr = 0;
				}
			}
		}
		return *this;
	}
	
	///ǿ������ת��
	///@param	rhs		��ת��������ָ�����
	///return	����ת���������ָ��
	template<class Y>
	static SharedPtr DynamicCast(const SharedPtrBase<Y>& rhs)
	{
		//return SharedPtr<T>(dynamic_cast<T*>(rhs.m_ptr));
		return SharedPtr(dynamic_cast<T*>(rhs.m_ptr));
	}
	
	///ǿ������ת��
	///@param[in]	p		��ת���Ķ���ָ��
	///return	����ת���������ָ��
	template<class Y>
	static SharedPtr DynamicCast(Y* p)
	{
		return SharedPtr(dynamic_cast<T*>(p));
	}
};

}

#endif
