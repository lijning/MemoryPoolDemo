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
//堆栈实现
//给一个极大的堆栈
//在此对堆栈内存池里实现快速存取
	size_t    _block;                                   //被分配的内存块的大小
	static const int MAX_BLOCKS = 10000;              //可创建对象数
	void*     stackPtr[MAX_BLOCKS];                      //存放废弃的内存块地址的数组    
	int       top;
	void*     base;                                      //base、limit保存大内存块地址范围
	void*     limit;
	void*     curr;                                      //内存块中可分配内存的首地址
};

#endif // !MEM_POOL
