#pragma once


template<size_t block_size, unsigned int block_count>
class SubMemPool {
	union FreeNode {
		FreeNode* pNext;
		char data[block_size];
	};

	//内存块结构体
	struct MemBlock {
		MemBlock *pNext;
		FreeNode data[block_count];//存放数据的内存空间
	};
	template<int ObjectSize, int NumofObjects>
	void* MemPool<ObjectSize, NumofObjects>::malloc() {
		if (freeNodeHeader == NULL) {      //无空闲节点，新建一个block，将block的每一个data空间都放进freelist
			MemBlock* newBlock = new MemBlock;
			newBlock->data[0].pNext = NULL; //设置内存块的第一个节点为空闲节点链表的最后一个
			for (int i = 1; i<NumofObjects; ++i)
				newBlock->data[i].pNext = &newBlock->data[i - 1];
			freeNodeHeader = &newBlock->data[NumofObjects - 1];
			newBlock->pNext = memBlockHeader;
		}
		//返回空节点闲链表的第一个节点
		void* freeNode = freeNodeHeader;
		freeNodeHeader = freeNodeHeader->pNext;
		return freeNode;
	}
};

class MemPool {
public:
	MemPool() = delete;
	MemPool(size_t max_handled_size_exponentiation);
	void* malloc(size_t sz);
};
