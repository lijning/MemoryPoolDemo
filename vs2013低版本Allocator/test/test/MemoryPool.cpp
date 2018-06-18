#include "stdafx.h"
#include "memorypool.h"

#include <iostream>

//申请一块连续的大内存，并将它的地址的范围保存在base和limit两个指针中，并将该内存的起始地址赋给curr指针。
mem_pool::mem_pool(size_t n, size_t sz) :top(0),base(nullptr),limit(nullptr),_block(sz){
	base = ::operator new(sz*MAX_BLOCKS);
	limit = static_cast<unsigned char*>(base)+ sz *MAX_BLOCKS;
	curr = base;
}

mem_pool::mem_pool()/*:mem_pool(1000,16384)*/{}//为了尽可能少的使用new(),将初始内存设置尽可能的大

void* mem_pool::allocate(size_t sz) {
	if (sz != _block) //超出内存块的内存设置
		return ::operator new(sz);

	
	//从初始大内存中分配内存
	if (curr < limit)
	{
		void* ptr = curr;
		curr = static_cast<unsigned char*>(curr)+sz;
		return ptr;
	}
	//使用stackPtr堆栈中回收的内存的地址分配内存
	if (top > 0)
	{
		return stackPtr[--top];
	}

	return NULL;
	
}
//stackPtr和top实现了堆栈的功能，将被回收的内存地址保存到堆栈的末尾
void mem_pool::deallocate(void *ptr, size_t sz) {
	if (sz != _block)
		::operator delete(ptr);
	else
		stackPtr[top++] = ptr;           //将欲回收的内存的地址压入statckPtr堆栈
	return;
}


mem_pool::~mem_pool() {
	if (base != nullptr)
		operator delete(base);
}
