#pragma once
template<size_t block_size, unsigned int block_count>
class SubMemPool {

};

class MemPool {
public:
	MemPool() = delete;
	MemPool(size_t max_handled_size_exponentiation);
	void* malloc(size_t sz);
};
