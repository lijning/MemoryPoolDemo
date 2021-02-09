#ifndef myallocator_h
#define myallocator_h
#include "stdafx.h"
#include "MemoryPool.h"
// memory pool based allocator
template <typename T>
class MyAllocator {
public:
	// naming tradition
	typedef T value_type;
	typedef T *pointer;
	typedef const T *const_pointer;
	typedef T &reference;
	typedef const T &const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	// for inner rebind michanics
	// used to transform allocator of T to tyep U
	template <typename U> struct rebind {
		typedef MyAllocator<U>
			other;
	};

	// constructors and destructors
	MyAllocator() = default;
	MyAllocator(const MyAllocator &) {}
	template <typename U> MyAllocator(const MyAllocator<U> &other) {}
	// explicitly note that assignment operator is not needed
	MyAllocator &operator=(const MyAllocator &) = delete;
	~MyAllocator() = default;

	// reuturn address of value_type
	static pointer address(reference r) {
		return &r;
	}
	static const_pointer address(const_reference cr) {
		return &cr;
	}

	// reutrn maximun number can be allocated
	static size_type max_size() {
		return std::numeric_limits<size_type>::max();
	}

	// operator used for compare
	bool operator==(const MyAllocator &) const {
		return true;
	}
	bool operator!=(const MyAllocator &) const {
		return false;
	}

	// implement by wrapping operator new and delete
	// allocation and deallocation
	static pointer allocate(size_type n) {
		// result will be copied to return value
		// use memory pool memp
		return static_cast<pointer>(memp.allocate(sizeof(T) * n));
	}
	static pointer allocate(size_type n, pointer ptr) {
		return allocate(n);
	}
	static void deallocate(pointer ptr, size_type n) {
		// use memory pool memp
		memp.deallocate(ptr, n);
	}

	// constrution valu_type and destruction
	// still use new, here we just need to call the constructor
	static void construct(pointer ptr, const value_type &t) {
		new(ptr)value_type(t);
	}
	static void destroy(pointer ptr) {
		ptr->~value_type();
	}

private:
	static mem_pool memp;
};

template <typename T> mem_pool MyAllocator<T>::memp;

#endif