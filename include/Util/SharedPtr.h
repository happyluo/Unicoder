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
///侵入式引用计数智能指针基类，实现对被管理指针的基本操作
template<typename T>
class SharedPtrBase
{
public:
	
	/// 定义元素类型别名
	typedef T element_type;
	
	///获取管理的指针
	T* Get() const
	{
		return m_ptr;
	}
	
	///获取管理的指针
	T* operator->() const
	{
		if (!m_ptr)
		{
			ThrowNullSharedPtrException(__FILE__, __LINE__);           
		}
		
		return m_ptr;
	}
	
	///获取被管理的指针所对应的对象
	T& operator*() const
	{
		if (!m_ptr)
		{
			ThrowNullSharedPtrException(__FILE__, __LINE__);           
		}
		
		return *m_ptr;
	}
	
	///检测当前对象是否有效
	operator bool() const
	{
		return m_ptr ? true : false;
	}
	
	///交换两个智能指针
	void Swap(SharedPtrBase& other)
	{
		std::swap(m_ptr, other.m_ptr);
	}
	
	///被共享的指针
	T *m_ptr;				

private:
	
	///抛出空指针异常
	///@param	file		发生异常的文件
	///@param	line	异常所发生的行
	void ThrowNullSharedPtrException(const char *file, int line) const;
};
	
template<typename T> inline void 
SharedPtrBase<T>::ThrowNullSharedPtrException(const char* file, int line) const
{
	throw NullSharedPtrException(file, line);
}

///判断两个指针是否相等
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

///判断两个指针是否不相等
template<typename T, typename U>
inline bool operator !=(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
	return !operator ==(lhs, rhs);
}

///判断左操作数是否小于右操作数
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

///判断左操作数是否小于等于右操作数
template<typename T, typename U>
inline bool operator <=(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
	return lhs < rhs || lhs == rhs;
}

///判断左操作数是否大于右操作数
template<typename T, typename U>
inline bool operator >(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
	return !(lhs < rhs || lhs == rhs);
}

///判断左操作数是否大于等于右操作数
template<typename T, typename U>
inline bool operator >=(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
	return !(lhs < rhs);
}
	
// class SharedPtr
///侵入式引用计数智能指针类，实现对指针本身及其所对应内存的管理
template<typename T>
class SharedPtr : public SharedPtrBase<T>
{
public:   
	///构造函数
	///@param	p	希望被管理的对象指针
	SharedPtr(T* p = 0)
	{
		this->m_ptr = p;
		
		if (this->m_ptr)
		{
			this->m_ptr->IncRef();
		}
	}

	///拷贝构造函数
	template<typename Y>
	SharedPtr(const SharedPtr<Y>& rhs)
	{
		this->m_ptr = rhs.m_ptr;
		
		if (this->m_ptr)
		{
			this->m_ptr->IncRef();
		}
	}
	
	///拷贝构造函数
	SharedPtr(const SharedPtr& rhs)
	{
		this->m_ptr = rhs.m_ptr;
		
		if (this->m_ptr)
		{
			this->m_ptr->IncRef();
		}
	}
	
	///析构函数
	~SharedPtr()
	{
		if (this->m_ptr)
		{			
			this->m_ptr->DecRef();
		}
		
		this->m_ptr = 0;
	}

	///赋值操作符
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
	
	///赋值操作符
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
	
	///赋值操作符
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
	
	///强制类型转换
	///@param	rhs		待转换的智能指针对象
	///return	返回转换后的智能指针
	template<class Y>
	static SharedPtr DynamicCast(const SharedPtrBase<Y>& rhs)
	{
		//return SharedPtr<T>(dynamic_cast<T*>(rhs.m_ptr));
		return SharedPtr(dynamic_cast<T*>(rhs.m_ptr));
	}
	
	///强制类型转换
	///@param[in]	p		待转换的对象指针
	///return	返回转换后的智能指针
	template<class Y>
	static SharedPtr DynamicCast(Y* p)
	{
		return SharedPtr(dynamic_cast<T*>(p));
	}
};

}

#endif
