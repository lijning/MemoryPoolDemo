#include "stdafx.h"
#include "memorypool.h"

#include <iostream>

//����һ�������Ĵ��ڴ棬�������ĵ�ַ�ķ�Χ������base��limit����ָ���У��������ڴ����ʼ��ַ����currָ�롣
mem_pool::mem_pool(size_t n, size_t sz) :top(0),base(nullptr),limit(nullptr),_block(sz){
	base = ::operator new(sz*MAX_BLOCKS);
	limit = static_cast<unsigned char*>(base)+ sz *MAX_BLOCKS;
	curr = base;
}

mem_pool::mem_pool()/*:mem_pool(1000,16384)*/{}//Ϊ�˾������ٵ�ʹ��new(),����ʼ�ڴ����þ����ܵĴ�

void* mem_pool::allocate(size_t sz) {
	if (sz != _block) //�����ڴ����ڴ�����
		return ::operator new(sz);

	
	//�ӳ�ʼ���ڴ��з����ڴ�
	if (curr < limit)
	{
		void* ptr = curr;
		curr = static_cast<unsigned char*>(curr)+sz;
		return ptr;
	}
	//ʹ��stackPtr��ջ�л��յ��ڴ�ĵ�ַ�����ڴ�
	if (top > 0)
	{
		return stackPtr[--top];
	}

	return NULL;
	
}
//stackPtr��topʵ���˶�ջ�Ĺ��ܣ��������յ��ڴ��ַ���浽��ջ��ĩβ
void mem_pool::deallocate(void *ptr, size_t sz) {
	if (sz != _block)
		::operator delete(ptr);
	else
		stackPtr[top++] = ptr;           //�������յ��ڴ�ĵ�ַѹ��statckPtr��ջ
	return;
}


mem_pool::~mem_pool() {
	if (base != nullptr)
		operator delete(base);
}
