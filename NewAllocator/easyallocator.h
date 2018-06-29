#pragma once
#include "mempool.h"
#include <memory>

template <typename T>
class EasyAllocator // Replace the std::allocator with it.
{
private:
	static MemPool<T> memp;

public:
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef T*        pointer;
	typedef const T*  const_pointer;
	typedef T&        reference;
	typedef const T&  const_reference;
	typedef T         value_type;

	EasyAllocator() = default;
	EasyAllocator(const EasyAllocator&) {};

	pointer	allocate(size_type n, const void * = 0) throw() {
		//std::allocator<T> alloc;
		// p = static_cast<pointer>(malloc(sizeof(T)*n));
		//pointer r = static_cast<pointer>(::operator new (sizeof(T)*n));
		//pointer s = alloc.allocate(n);

		pointer q = (memp.malloc(n));//my_proxy¿É¶Á¡£ 

		return q;
	}

	void	deallocate(pointer p, size_type n) {
		if(NULL!=p){
			memp.free(p, n);
			//free(p);
		}
		else {
			exit(-1);
		}
	}

	pointer		address(reference x) const { return &x; }
	const_pointer	address(const_reference x) const { return &x; }
	EasyAllocator<T>&	operator=(const EasyAllocator&) { return *this; }

	static void	construct(pointer p, const T& val)	
	{
		new((pointer)p) value_type(val);
	}
	static void	destroy(pointer p)
	{ 
		p->~T(); 
	}

	size_type	max_size() const throw()
	{ return std::numeric_limits<size_type>::max(); }

	template <class U>
	struct rebind { typedef EasyAllocator<U> other; };

	template <class U>
	EasyAllocator(const EasyAllocator<U>&) {}

	template <class U>
	EasyAllocator& operator=(const EasyAllocator<U>&) { return *this; }
};
// globals
template <class T, class U>
bool operator==(const EasyAllocator<T>&,
	const EasyAllocator<U>&) throw() {
	return true;
}
template <class T, class U>
bool operator!=(const EasyAllocator<T>&,
	const EasyAllocator<U>&) throw() {
	return false;
}
template <typename T>
MemPool<T> EasyAllocator<T>::memp;// Initialize the static member 'memp'.