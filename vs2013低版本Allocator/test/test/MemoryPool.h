#pragma once

#ifndef MEM_POOL
#define MEM_POOL


class mem_pool {
public:
	mem_pool();
	mem_pool(size_t n, size_t sz);
	void* allocate(size_t sz);
	void deallocate(void *ptr, size_t = 0);
	~mem_pool();

private:
//��ջʵ��
//��һ������Ķ�ջ
//�ڴ˶Զ�ջ�ڴ����ʵ�ֿ��ٴ�ȡ
	size_t    _block;                                   //��������ڴ��Ĵ�С
	static const int MAX_BLOCKS = 10000;              //�ɴ���������
	void*     stackPtr[MAX_BLOCKS];                      //��ŷ������ڴ���ַ������    
	int       top;
	void*     base;                                      //base��limit������ڴ���ַ��Χ
	void*     limit;
	void*     curr;                                      //�ڴ���пɷ����ڴ���׵�ַ
};

#endif // !MEM_POOL
