#pragma once

template<size_t NodeData_sz, unsigned int Block_NodeCount=768>
class SubMemPool { // Each MemPool consist of several SubPools.
private:

	union FreeNode {
		FreeNode* pNext;
		char data[NodeData_sz];
	};

	struct MemBlock {
		MemBlock *pNext;
		FreeNode nodes[Block_NodeCount];
	};

	FreeNode* freeNodeHeader;
	MemBlock* memBlockHeader;

public:
	SubMemPool()
	{
		freeNodeHeader = NULL;
		memBlockHeader = NULL;
	}

	~SubMemPool() {
		MemBlock* ptr;
		while (NULL!=memBlockHeader){
			ptr = memBlockHeader->pNext;
			delete memBlockHeader;
			memBlockHeader = ptr;
		}
	}
	void* malloc() {
		if (freeNodeHeader == NULL) {
			MemBlock* pBlock = new MemBlock();
			pBlock->nodes[0].pNext = NULL;
			for (int i = 1; i < Block_NodeCount; i++) {
				pBlock->nodes[i].pNext = &pBlock->nodes[i-1];
			}
			freeNodeHeader= &pBlock->nodes[Block_NodeCount - 1];
			pBlock->pNext = memBlockHeader;
			memBlockHeader = pBlock;
		}
		void* pFN = freeNodeHeader;
		freeNodeHeader = freeNodeHeader->pNext;
		return pFN;
	}
	void free(void* p) {
		FreeNode* pFN = (FreeNode*)p;
		pFN->pNext = freeNodeHeader;
		freeNodeHeader = pFN;
	}
};

template<typename Ty>
class MemPool {
	// MemPool determine which Submemp to use.
private:
	const size_t TSZ;
	const size_t MAX_CNT;
	SubMemPool<4 * sizeof(Ty),16> mp4;
	SubMemPool<1024 * sizeof(Ty)> mp1024;
	SubMemPool<2048 * sizeof(Ty),512> mp2048;
	SubMemPool<4096 * sizeof(Ty)> mp4096;
	SubMemPool<6144 * sizeof(Ty)> mp6144;
	SubMemPool<8192 * sizeof(Ty)> mp8192;

public:
	MemPool() : TSZ(sizeof(Ty)), MAX_CNT(8192) {}
	Ty* _cdecl malloc(size_t szn) {
		if (0 < szn && szn < 4)return static_cast<Ty*>(mp4.malloc());
		else if (szn < 1024)return static_cast<Ty*> (mp1024.malloc());
		else if (szn < 2048)return static_cast<Ty*>(mp2048.malloc());
		else if (szn < 4096)return static_cast<Ty*>(mp4096.malloc());
		else if (szn < 6144)return static_cast<Ty*>(mp6144.malloc());
		else if (szn < MAX_CNT)return static_cast<Ty*>(mp8192.malloc());
		else
			return static_cast<Ty*>(::operator new(szn*sizeof(Ty)));
	}

	void free(Ty* p,size_t szn) {// 此处szn是Ty的数量，而非其占用的内存空间。. 
		if (0 < szn && szn < 4)
			mp4.free(p);
		else if (szn < 1024)mp1024.free(p);
		else if (szn < 2048)mp2048.free(p);
		else if (szn < 4096)mp4096.free(p);
		else if (szn < 6144)mp6144.free(p);
		else if (szn < MAX_CNT)mp8192.free(p);
		else
			::operator delete(p);
	}
};
