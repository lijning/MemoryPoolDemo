#pragma once


template<size_t block_size, unsigned int block_count>
class SubMemPool {
	union FreeNode {
		FreeNode* pNext;
		char data[block_size];
	};

	//�ڴ��ṹ��
	struct MemBlock {
		MemBlock *pNext;
		FreeNode data[block_count];//������ݵ��ڴ�ռ�
	};
	template<int ObjectSize, int NumofObjects>
	void* MemPool<ObjectSize, NumofObjects>::malloc() {
		if (freeNodeHeader == NULL) {      //�޿��нڵ㣬�½�һ��block����block��ÿһ��data�ռ䶼�Ž�freelist
			MemBlock* newBlock = new MemBlock;
			newBlock->data[0].pNext = NULL; //�����ڴ��ĵ�һ���ڵ�Ϊ���нڵ���������һ��
			for (int i = 1; i<NumofObjects; ++i)
				newBlock->data[i].pNext = &newBlock->data[i - 1];
			freeNodeHeader = &newBlock->data[NumofObjects - 1];
			newBlock->pNext = memBlockHeader;
		}
		//���ؿսڵ�������ĵ�һ���ڵ�
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
